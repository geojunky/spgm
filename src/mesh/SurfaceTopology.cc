/*
 * =====================================================================================
 * Scalable PaleoGeomorphology Model (SPGM)
 *
 * Copyright (C) 2014 Rakib Hassan (rakib.hassan@sydney.edu.au)
 *
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * ===================================================================================== 
 */
/*
 * =====================================================================================
 *
 *       Filename:  SurfaceTopology.cc
 *
 *    Description:  This class implements the mechanics described in sections 2 and 3
 *                  of Braun et al. (2013)
 *
 *        Version:  1.0
 *        Created:  24/02/14 13:36:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au) 
 *
 * =====================================================================================
 */
#include <SurfaceTopology.hh>
#include <ScalarField.hh>
#include <Timer.hh>
#include <Log.hh>
#include <tinyxml2.hh>

namespace src { namespace mesh {
    using namespace std;
    using namespace src::util;
    using namespace src::parser;
    using namespace src::parser::tinyxml2;
    
    const int SurfaceTopology::DIRICHLET                = 1;
    const int SurfaceTopology::NEUMANN                  = 2;
    const int SurfaceTopology::CYCLIC                   = 3;
    const int SurfaceTopology::INVALID                  = -1;
    const int SurfaceTopology::ORPHAN                   = -2;

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: SurfaceTopology
     * Description:  Constructor initializes mesh and network.
     *--------------------------------------------------------------------------------------
     */
    SurfaceTopology::SurfaceTopology(Config *c)
    :m_config(c)
    {
        /*-----------------------------------------------------------------------------
         * Read parameters 
         *-----------------------------------------------------------------------------*/
        m_meshFileName = m_config->PString("fileName");
        
        /*-----------------------------------------------------------------------------
         * Read optional parameters 
         *-----------------------------------------------------------------------------*/
        m_smoothing             = m_config->PBool("smoothing");
        m_smoothingFactor       = float(m_config->PDouble("smoothingFactor"));
        m_smoothingIterations   = m_config->PInt("smoothingIterations");
        m_meshStartTime         = 0;

        m_rawGeometry = ReadMeshGeometry(&m_nMeshPoints);
        
        /*-----------------------------------------------------------------------------
         * Initialize kd-tree for spatial queries
         *-----------------------------------------------------------------------------*/
        m_kdTree = NULL;
        InitializeKdTree();

        /*-----------------------------------------------------------------------------
         * Initialize triangulation
         *-----------------------------------------------------------------------------*/
        printf("[Delaunay Triangulation: ");
        Timer tTriangulationBegin;
        m_triangulator = new Triangulator(m_nMeshPoints, m_rawGeometry, 
                                   Triangulator::Triangulator_TriangleIndices       | 
                                   Triangulator::Triangulator_TriangleNeighbours    | 
                                   Triangulator::Triangulator_VoronoiVertices       | 
                                   Triangulator::Triangulator_VoronoiSides          |
                                   Triangulator::Triangulator_VoronoiCellAreas      |
                                   Triangulator::Triangulator_NodeNeighbours);
        Timer tTriangulationEnd; 
        printf("%lf s]\n", Timer::Elapsed(tTriangulationBegin, tTriangulationEnd));
    
        /*-----------------------------------------------------------------------------
         * Validate boundary conditions 
         *-----------------------------------------------------------------------------*/
        ValidateBoundaryConditions();

        /*-----------------------------------------------------------------------------
         * Compute average cell-area
         *-----------------------------------------------------------------------------*/
        const float *surfaceArea = GetVoronoiCellAreas();
        const int *hull = GetHull();
        m_averageCellArea = 0.;
        int len = GetNMeshPoints();
        int count = 0;
        for(int i=0; i<len; i++)
        {
            if(!hull[i]) 
            {
                m_averageCellArea += surfaceArea[i];
                count++;
            }
        }
        m_averageCellArea /= (float)(count);

#ifdef DEBUG
        PrintMeshDetails();
#endif

        /*-----------------------------------------------------------------------------
         * Allocate memory for flow-network related structures
         *-----------------------------------------------------------------------------*/
        m_receivers                     = new int[m_nMeshPoints];
        m_receiversSillCorrected        = new int[m_nMeshPoints];
        m_donorCounts                   = new int[m_nMeshPoints];
        m_donors                        = new int*[m_nMeshPoints];
        m_donorsStorage                 = new int[m_nMeshPoints];
        m_stack                         = new int[m_nMeshPoints];
        m_catchmentIds                  = new int[m_nMeshPoints];
        m_z0                            = new float[m_nMeshPoints];
        m_zp                            = new float[m_nMeshPoints];
        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_z0[i] = m_zp[i] = m_rawGeometry[i][2];
        }

        printf("[Initializing Network: ");Timer tInitNetworkBegin;
        InitializeNetwork();
        Timer tInitNetworkEnd; printf("%lf s]\n", Timer::Elapsed(tInitNetworkBegin, tInitNetworkEnd));

#ifdef DEBUG
        PrintNetwork();
#endif
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: ~SurfaceTopology
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    SurfaceTopology::~SurfaceTopology()
    {
        delete m_kdTree;
        delete m_triangulator;

        delete [] m_rawGeometry[0];
        delete [] m_rawGeometry;

        delete [] m_z0;
        delete [] m_zp;
        delete [] m_receivers;
        delete [] m_receiversSillCorrected;
        delete [] m_donorCounts;
        delete [] m_donors;
        delete [] m_donorsStorage;
        delete [] m_stack;
        delete [] m_catchmentIds;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: InitializeKdTree
     * Description:  This function initializes the member kd-tree to facilitate spatial
     *               queries needed for interpolating values from the unstructured mesh
     *               to a regular grid. The underlying assumption is that the spatial 
     *               coordinates are fixed in time.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::InitializeKdTree()
    {
        if(m_kdTree) delete m_kdTree;

        m_kdTree = new KdTree(m_nMeshPoints);

        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_kdTree->Add(m_rawGeometry[i], i);
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: InitializeNetwork
     * Description:  This function computes for every node their receiver, donor's list 
     *               their order in the stack, the catchment they belong to and finally 
     *               assigns local minima to their closest catchment that drains to an 
     *               outlet node. This function has been implemented as described in 
     *               section 2 of Braun et al. (2013). See section 6 of the latter for 
     *               more details on treatment of local minima.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::InitializeNetwork()
    {
        unsigned int *numNeighbours = m_triangulator->GetNumNeighbours();
        unsigned int **neighbours    = m_triangulator->GetNeighbours();

        /*-----------------------------------------------------------------------------
         * Find receivers. 
         *-----------------------------------------------------------------------------*/
        #pragma omp parallel for
        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_receivers[i] = i;

            if(B(i)==DIRICHLET) continue; /* Carrying on if it's a base node. */

            int lowestNeighbour = i;
            for(unsigned int j=0; j<numNeighbours[i]; j++)
            {
                int neighbour = neighbours[i][j];
                if(Z(neighbour) < Z(lowestNeighbour)) 
                    lowestNeighbour = neighbour;
            }
            m_receivers[i] = lowestNeighbour;
        }

        /*-----------------------------------------------------------------------------
         * Initialize donor's list
         *-----------------------------------------------------------------------------*/
        for(int i=0; i<m_nMeshPoints; i++) 
        {
            m_donorCounts[i] = 0;
            m_donors[i] = NULL;
            m_donorsStorage[i] = 0;
        }

        for(int i=0; i<m_nMeshPoints; i++)
        {
            if(R(i) != i) /* Has a receiver other than itself */
            {
                m_donorCounts[R(i)]++;
            }
        }

        for(int i=0, sum=0; i<m_nMeshPoints; i++)
        {
            if(Dn(i))
            {
                m_donors[i] = &(m_donorsStorage[sum]);
                sum += Dn(i);
            }
        }


        /*-----------------------------------------------------------------------------
         * Populate Donors-list arrays
         *-----------------------------------------------------------------------------*/
        {
            int *donorsCount = new int[m_nMeshPoints];

            memset(donorsCount, 0, sizeof(int)*m_nMeshPoints);
            
            for(int i=0; i<m_nMeshPoints; i++)
            {
                if(R(i) != i) /* Has a receiver other than itself */
                {
                    m_donors[R(i)][donorsCount[R(i)]++] = i;
                }
            }

            delete [] donorsCount;
        }
                
        /*-----------------------------------------------------------------------------
         * Initialize stack
         *-----------------------------------------------------------------------------*/
        int index = 0;
        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_stack[i] = INVALID;
            m_catchmentIds[i] = ORPHAN;
        }
        
        for(int i=0; i<m_nMeshPoints; i++)
        {
            if(B(i)==DIRICHLET || (R(i)==i)) 
            {
                if(B(i)==DIRICHLET) m_catchmentIds[i] = i;

                if(Dn(i)) InitializeStack(&index, i, m_catchmentIds[i]);
            }
        }
        
        /*-----------------------------------------------------------------------------
         * Initialize sillCorrected receivers-array
         *-----------------------------------------------------------------------------*/
        memcpy(m_receiversSillCorrected, m_receivers, sizeof(int)*m_nMeshPoints);
        while(CountOrphanNodes())
        {
            for(int i=0; i<m_nMeshPoints; i++)
            {
                if(C(i)==ORPHAN)
                {
                    int sill = INVALID;
                    float sillHeight = numeric_limits<float>::max();
                    for(unsigned int j=0; j<numNeighbours[i]; j++)
                    {
                        int neighbour = neighbours[i][j];
                        if(C(neighbour)==ORPHAN) continue;

                        if(sillHeight > Z(neighbour))
                        {
                            sillHeight = Z(neighbour);
                            sill = neighbour;
                        }
                    }

                    if(sill != INVALID)
                    {
                        m_receiversSillCorrected[i] = sill;
                        PropagateCatchmentTagUpstream(i, C(sill));
                    }
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Populate catchment list
         *-----------------------------------------------------------------------------*/
        m_catchments.erase(m_catchments.begin(), m_catchments.end());
        for(int i=0; i<m_nMeshPoints; i++) m_catchments.insert(C(i));
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: CountOrphanNodes
     * Description:  Counts the number of nodes that are yet to be assigned to a catchment, 
     *               i.e. nodes that have a catchmentId set to ORPHAN.
     *--------------------------------------------------------------------------------------
     */
    int SurfaceTopology::CountOrphanNodes()
    {
        int result = 0;

        for(int i=0; i<m_nMeshPoints; i++) if(C(i)==ORPHAN) result++;
        
        return result;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: InitializeStack
     * Description:  Initializes the stack-order recursively.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::InitializeStack(int *index, int node, int catchmentId)
    {
        m_stack[*index] = node;
        m_catchmentIds[node] = catchmentId;
        *index += 1;

        for(int i=0; i<Dn(node); i++)
        {
            int donor = D(node)[i];

            m_catchmentIds[donor] = catchmentId;
            InitializeStack(index, donor, catchmentId);
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: PropagateCatchmentTagUpstream
     * Description:  Propagates a catchment-id upstream recursively.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::PropagateCatchmentTagUpstream(int node, int catchmentId)
    {
        m_catchmentIds[node] = catchmentId;
        for(int i=0; i<Dn(node); i++)
        {
            int donor = D(node)[i];
            
            m_catchmentIds[donor] = catchmentId;
            PropagateCatchmentTagUpstream(donor, catchmentId);
        }
    }
    

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: ReadTextMesh
     * Description:  Reads a mesh file in txt format
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::ReadTextMesh(int *nMeshPoints, float ***points, float ***pointsSorted)
    {
        ifstream ifs;
        string line;
        int npt=0;

        ifs.open(m_meshFileName.c_str());

        if(!ifs.is_open())
        {
            cerr << "Error: mesh-file could not be opened.." << endl;
            exit(EXIT_FAILURE);
        }
        
        /* Get number of mesh points */
        getline(ifs, line);
        npt = atoi(line.c_str());
        *points                = new float*[npt];
        (*points)[0]           = new float[npt*4];
        *pointsSorted          = new float*[npt];
        (*pointsSorted)[0]     = new float[npt*4];

        for (int i=0; i<npt; i++)
        {
            (*points)[i]       = (*points)[0]+i*4;
            (*pointsSorted)[i] = (*pointsSorted)[0]+i*4;
        }

        /*-----------------------------------------------------------------------------
         * Initialize bounding-box 
         *-----------------------------------------------------------------------------*/
        for( int i=0; i<2; i++ )
        {
            m_upper[i] = -numeric_limits<float>::max();
            m_lower[i] = numeric_limits<float>::max();
        }

        int count=0;
        int dirichletCount=0;
        while(getline(ifs, line))
        {
            sscanf(line.c_str(), "%f %f %f %f", 
                   &((*points)[count][0]), 
                   &((*points)[count][1]), 
                   &((*points)[count][2]),
                   &((*points)[count][3]));
            
            if((*points)[count][3] > 2 || (*points)[count][3] < 0)
            {
                cerr << "Error: BC column in mesh-file has values other than [0,1,2]" << endl;
                exit(EXIT_FAILURE);
            }
            
            /* Update bounding-box coords */
            if((*points)[count][0]>m_upper[0]) m_upper[0] = (*points)[count][0];
            if((*points)[count][1]>m_upper[1]) m_upper[1] = (*points)[count][1];
            if((*points)[count][0]<m_lower[0]) m_lower[0] = (*points)[count][0];
            if((*points)[count][1]<m_lower[1]) m_lower[1] = (*points)[count][1];

            if(((int)(*points)[count][3])==DIRICHLET) dirichletCount++;
            count++;
        }
        ifs.close();

        if(dirichletCount==0)
        {
            cerr << "Error: No Dirichlet-nodes found. Aborting.." << endl;
            exit(EXIT_FAILURE);
        }        
        
        *nMeshPoints = npt;
    }
    

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: ReadVTUMesh
     * Description:  Reads a mesh in vtu format
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::ReadVTUMesh(int *nMeshPoints, float ***points, float ***pointsSorted)
    {
        XMLDocument xmlDoc;
        
        if(xmlDoc.LoadFile(m_meshFileName.c_str()) == tinyxml2::XML_NO_ERROR){}
        else
        {
            LogError(cout << "Error loading xml ifle: " << m_meshFileName << endl);
            exit(EXIT_FAILURE);
        }

        vector<float> bcVec, xVec, yVec, hVec, tVec;
        vector<int>order;
        /*-----------------------------------------------------------------------------
         * Parse xml file
         *-----------------------------------------------------------------------------*/
        {
            /* Get number of points */
            tinyxml2::XMLElement *piece = xmlDoc.FirstChildElement("VTKFile")->
                                            FirstChildElement("UnstructuredGrid")->
                                            FirstChildElement("Piece");
            *nMeshPoints  = atoi(piece->Attribute("NumberOfPoints"));
            
            /* Resize vectors */
            bcVec.resize(*nMeshPoints);
            xVec.resize(*nMeshPoints);
            yVec.resize(*nMeshPoints); 
            hVec.resize(*nMeshPoints);
            order.resize(*nMeshPoints);
            tVec.resize(*nMeshPoints);

            /* bc */
            tinyxml2::XMLElement *pointData = piece->FirstChildElement("PointData");
            for(tinyxml2::XMLElement *dataArray = pointData->
                                                  FirstChildElement("DataArray"); dataArray;
                dataArray = dataArray->NextSiblingElement())
            {
                if(string(dataArray->Attribute("Name"))=="bc")
                {
                    stringstream ss (dataArray->GetText());
                    for(int i=0; i<*nMeshPoints; i++)
                    {
                        float bc;

                        ss >> bc;
                        bcVec[i] = bc;
                    }                    
                }
                
                if(string(dataArray->Attribute("Name"))=="order")
                {
                    stringstream ss (dataArray->GetText());
                    for(int i=0; i<*nMeshPoints; i++)
                    {
                        int o;

                        ss >> o;
                        order[i] = o;
                    }                    
                }
                
                if(string(dataArray->Attribute("Name"))=="t")
                {
                    stringstream ss (dataArray->GetText());
                    for(int i=0; i<*nMeshPoints; i++)
                    {
                        float t;

                        ss >> t;
                        tVec[i] = t;
                    }                    
                }                
            }

            /* Read points */
            {
                tinyxml2::XMLElement *pointsDataArray = piece->FirstChildElement("Points")->
                                                        FirstChildElement("DataArray");
                stringstream ss (pointsDataArray->GetText());
                for(int i=0; i<*nMeshPoints; i++)
                {
                    float x, y, h;

                    ss >> x;
                    ss >> y;
                    ss >> h;
                    
                    xVec[i] = x;
                    yVec[i] = y;
                    hVec[i] = h;
                }                   
            }
        }
        
        /* Allocate return arrays */
        int npt=*nMeshPoints;

        *points                = new float*[npt];
        (*points)[0]           = new float[npt*4];
        *pointsSorted          = new float*[npt];
        (*pointsSorted)[0]     = new float[npt*4];

        for (int i=0; i<npt; i++)
        {
            (*points)[i]       = (*points)[0]+i*4;
            (*pointsSorted)[i] = (*pointsSorted)[0]+i*4;
        }

        /*-----------------------------------------------------------------------------
         * Initialize bounding-box 
         *-----------------------------------------------------------------------------*/
        for( int i=0; i<2; i++ )
        {
            m_upper[i] = -numeric_limits<float>::max();
            m_lower[i] = numeric_limits<float>::max();
        }

        int count=0;
        int dirichletCount=0;
        for(count=0; count<npt; count++)
        {
            (*points)[count][0] = xVec [order[count]];
            (*points)[count][1] = yVec [order[count]];
            (*points)[count][2] = hVec [order[count]];
            (*points)[count][3] = bcVec[order[count]];
            
            if((*points)[count][3] > 2 || (*points)[count][3] < 0)
            {
                cerr << "Error: BC column in mesh-file has values other than [0,1,2]" << endl;
                exit(EXIT_FAILURE);
            }
            
            /* Update bounding-box coords */
            if((*points)[count][0]>m_upper[0]) m_upper[0] = (*points)[count][0];
            if((*points)[count][1]>m_upper[1]) m_upper[1] = (*points)[count][1];
            if((*points)[count][0]<m_lower[0]) m_lower[0] = (*points)[count][0];
            if((*points)[count][1]<m_lower[1]) m_lower[1] = (*points)[count][1];

            if(((int)(*points)[count][3])==DIRICHLET) dirichletCount++;
        }

        if(dirichletCount==0)
        {
            cerr << "Error: No Dirichlet-nodes found. Aborting.." << endl;
            exit(EXIT_FAILURE);
        }   

        m_meshStartTime = tVec[0];
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: ReadMeshGeometry
     * Description:  Reads mesh-file
     *--------------------------------------------------------------------------------------
     */
    float **SurfaceTopology::ReadMeshGeometry(int *nMeshPoints)
    {
        int npt;
        float **points;
        float **pointsSorted;
        
        vector<string> nameTokens = split(m_meshFileName, '.');
        if(nameTokens.size())
        {
            if(nameTokens[nameTokens.size()-1] == "txt") 
                ReadTextMesh(nMeshPoints, &points, &pointsSorted);
            else if(nameTokens[nameTokens.size()-1] == "vtu")
                ReadVTUMesh(nMeshPoints, &points, &pointsSorted);
            else goto err;
        }
        else goto err;
        
        npt = *nMeshPoints;
        /*-----------------------------------------------------------------------------
         * Process geometry
         *-----------------------------------------------------------------------------*/
        {
            /*-----------------------------------------------------------------------------
             * Apply smoothing 
             *-----------------------------------------------------------------------------*/
            if(m_smoothing)
            {
                Triangulator t(npt, points, 
                               Triangulator::Triangulator_TriangleIndices       | 
                               Triangulator::Triangulator_TriangleNeighbours    | 
                               Triangulator::Triangulator_VoronoiVertices       | 
                               Triangulator::Triangulator_VoronoiSides          |
                               Triangulator::Triangulator_VoronoiCellAreas      |
                               Triangulator::Triangulator_NodeNeighbours);
                unsigned int *nnbr  = t.GetNumNeighbours();
                unsigned int **nbr  = t.GetNeighbours();
                int *smoothingHull  = t.GetHull();

                cerr << "[Smooth mesh: ";
                Timer tSmoothBegin;
                
                for( int iter=0; iter<m_smoothingIterations; iter++ )
                {
                    for( int i=0; i<npt; i++ )
                    {
                        float ab[2] = {0};
                        //if( ((int)points[i][3]) != DIRICHLET )
                        if( !smoothingHull[i] )
                        {
                            for( unsigned int j=0; j<nnbr[i]; j++ )
                            {
                                unsigned int cnbr = nbr[i][j];
                                ab[0] += points[cnbr][0] - points[i][0];
                                ab[1] += points[cnbr][1] - points[i][1];
                            }
                        
                            ab[0] = ab[0] * m_smoothingFactor / (float(nnbr[i]));
                            ab[1] = ab[1] * m_smoothingFactor / (float(nnbr[i]));
                        
                            points[i][0] += ab[0];
                            points[i][1] += ab[1];
                        }
                    }
                }
                Timer tSmoothEnd; 
                printf("%lf s]\n", Timer::Elapsed(tSmoothBegin, tSmoothEnd));
            }

            /*-----------------------------------------------------------------------------
             * Re-order nodes so that the Dirichlet nodes are listed after all other nodes
             *-----------------------------------------------------------------------------*/
            int countForward = 0;
            int countBackward = npt-1;
            
            m_originalOrder.resize(npt);
            
            for (int i=0; i<npt; i++)
            {
                if(((int)points[i][3])!=1)
                {
                    memcpy(pointsSorted[countForward], points[i], sizeof(float)*4);
                    m_originalOrder[i] = countForward;
                    countForward++;
                }
                else
                {
                    memcpy(pointsSorted[countBackward], points[i], sizeof(float)*4);
                    m_originalOrder[i] = countBackward;
                    countBackward--;
                }
            }

            delete [] points[0];
            delete [] points;

            return pointsSorted;
        }
        
err:    LogError(cout << "Invalid mesh file: " << m_meshFileName << ". Format must be .txt or .vtu " << endl);
        exit(EXIT_FAILURE);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: UpdateZ
     * Description:  Individual processes call this function with net elevation change which
     *               is integrated in this class.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::UpdateZ(ScalarField<float> *z) const
    {
        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_rawGeometry[i][2] += (*z)(i);
        }
    }

    void SurfaceTopology::SavePreviousTimestep() const
    {
        for(int i=0; i<m_nMeshPoints; i++)
        {
            m_zp[i] = m_rawGeometry[i][2];
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: UpdateNetwork
     * Description:  This function is called every time-step and internally it calls
     *               InitializeNetwork()
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::UpdateNetwork()
    {
        InitializeNetwork();
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: PrintMeshDetails
     * Description:  Print triangulation-related attributes
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::PrintMeshDetails()
    {
        if(m_triangulator)
        {
            cout << m_triangulator << endl;

            unsigned int **triIndices = m_triangulator->GetTriangleIndices();
            long int numTriangles = m_triangulator->GetNumTriangles();
            for(int i=0; i<numTriangles; i++)
            {
                printf("%d %d %d\n", triIndices[i][0], triIndices[i][1], triIndices[i][2]);
            }

            int numVoronoiVertices = m_triangulator->GetNumVoronoiVertices();
            VSite *vv = m_triangulator->GetVoronoiVertices();

            for(int i=0; i<numVoronoiVertices; i++)
            {
                printf("%lf %lf\n", vv[i].m_coord[0], vv[i].m_coord[1]);
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: ValidateBoundaryConditions
     * Description:  Validate boundary conditions, making sure all boundary nodes fall on 
     *               the convex hull.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::ValidateBoundaryConditions()
    {
        if(m_triangulator == NULL) return;
        
        int *hull = m_triangulator->GetHull();
        for(int i=0; i<m_nMeshPoints; i++)
        {
            int bci = (int)B(i);

            if(bci && (!hull[i]))
            {
                cerr << "Error: BC nodes must lie on the convex hull of the mesh.." << endl;
                //exit(EXIT_FAILURE);
            }
        }
        
        m_triangulator->ComputeBound(&(m_lower[0]), &(m_lower[1]), &(m_upper[0]), &(m_upper[0]));

        //TODO: Check if Neumann BCs are valid edges in the triangulated mesh.
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: GetBounds
     * Description:  Get spatial bounds of the mesh
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::GetBounds(vector<float> &upper, vector<float> &lower) const
    {
        upper.clear();
        lower.clear();

        for(int i=0; i<2; i++)
        {
            upper.push_back(m_upper[i]);
            lower.push_back(m_lower[i]);
        }
    }

    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: InterpolateToRegularmesh
     * Description:  
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::InterpolateToRegularmesh(RegularMesh *rm, vector<float> &field) const
    {
        double radius = sqrt(rm->m_dx*rm->m_dx + rm->m_dy*rm->m_dy);

        int nx = rm->m_nx;
        int ny = rm->m_ny;
        
        #pragma omp parallel for
        for(int i=0; i<nx; i++)
        {
            for(int j=0; j<ny; j++)
            {
                vector<float> distance;
                vector<int> id;

                float pos[2] = {float(rm->m_x1a[i]), float(rm->m_x2a[j])};
                
                m_kdTree->QueryBallPoint(pos, radius, &distance, &id);
                while(distance.size()==0)
                {
                    m_kdTree->QueryBallPoint(pos, radius, &distance, &id);
                    radius*=2;
                }

                rm->m_values(i,j) = 0.;
                /* IDW interpolation */
                double distInvSum = 0.;
                double distInv = 0;
                double weightedVal = 0;
                int nn = distance.size();
                int foundCoincidentNode = 0;
                for(int k=0; k<nn; k++) 
                {
                    if(distance[k]==0)
                    {
                        rm->m_values(i,j) = field[id[k]];
                        foundCoincidentNode = 1;
                        break;
                    }
                    distInv = 1./distance[k];
                    distInvSum += distInv;
                    weightedVal += distInv * field[id[k]];
                }
                
                if(foundCoincidentNode) continue;
                
                /* Assign interpolated value */
                rm->m_values(i,j) = weightedVal / distInvSum;
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: PrintNetwork
     * Description:  Prints network
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::PrintNetwork()
    {
        int bcCount = 0;
        int locMinCount = 0;
        int DnSum = 0;
        printf("Network Details:\n");
        printf("----------------\n\n");

        for(int i=0; i<m_nMeshPoints; i++) 
        {
            if(B(i)) bcCount++;
            if((R(i)==i) && (!B(i))) locMinCount++;
            if(Dn(i)) DnSum += Dn(i);
        }
        
        printf ("Mesh node-count    : %d\n", m_nMeshPoints);
        printf ("Sum of donor-count : %d\n", DnSum);
        printf ("BC count           : %d\n", bcCount);
        printf ("Local minima-count : %d\n", locMinCount);
        printf ("Catchment-count    : %d\n", int(m_catchments.size()));

        printf("\nReceivers:\n");
        for(int i=0; i<m_nMeshPoints; i++)
        {
            printf("\t%d -> %d\n", i, R(i));
        }
        
        printf("\nDonors-list:\n");
        for(int i=0; i<m_nMeshPoints; i++)
        {
            if(Dn(i))
            {
                printf("\tDonor-count: %d\n", Dn(i));
                printf("\t %d <- [", i);
                for(int j=0; j<Dn(i); j++)
                {
                    if(j<(Dn(i)-1))
                        printf("%d, ", D(i)[j]);
                    else
                        printf("%d]\n", D(i)[j]);
                }
            }
        }
        
        printf("\nStack and height:\n");
        for(int i=0; i<m_nMeshPoints; i++)
        {
            if((S(i)!=INVALID) && (!Dn(i))) printf("\t %d* %f\n", S(i), Z(S(i))); /*hill-top nodes*/
            else if(S(i)!=INVALID) printf("\t %d %f\n", S(i), Z(S(i)));
            else printf("\t %d\n", S(i) );
        }
        
        printf("\nCatchment IDs:\n");
        for(set<int>::iterator sit=m_catchments.begin(); 
            sit!=m_catchments.end(); sit++)
        {
            printf("\t %d\n", *sit);
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopology
     *      Method:  SurfaceTopology :: PrintNode
     * Description:  Print an individual node
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopology::PrintNode(int index) const
    {
        cout << "[Node : " << index << endl;
        cout << "\t\t H: " << Z(index) << endl;
        cout << "\t\t Donor(s): ";
        for(int i=0; i<Dn(index); i++) cout << D(index)[i] << ((i==Dn(index)-1)?"":", ");
        cout << endl;
        cout << "\t\t Donor(s) H: ";
        for(int i=0; i<Dn(index); i++) cout << Z(D(index)[i]) << ((i==Dn(index)-1)?"":", ");
        cout << endl;
        cout << "\t\t Receiver: " << R(index) << endl;
        cout << "\t\t Receiver H: " << Z(R(index)) << endl;
        cout << "\t\t Sill Receiver: " << SR(index) << endl;
        cout << "\t\t Catchment ID: " << C(index) << endl;
        cout << "\t\t BC: " << B(index) << endl;
        cout << "\t\t Hull: " << GetHull()[index] << "]" << endl;
    }
}}

