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
 *       Filename:  SurfaceTopologyOutput.cc
 *
 *    Description:  Implements output routines
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:02:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)         
 *
 * =====================================================================================
 */
#include <SurfaceTopologyOutput.hh>
#include <Timer.hh>

namespace src { namespace mesh {
using namespace std;
using namespace src::util;

    const float SCALAR = 1;

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: SurfaceTopologyOutput
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    SurfaceTopologyOutput::SurfaceTopologyOutput(const Model *m, Config *c)
    :m_model(m),
    m_config(c),
    m_surfaceTopology(m_model->GetSurfaceTopology())
    {
        if(m_surfaceTopology == NULL) assert(0);        
        /*-----------------------------------------------------------------------------
         * Read paramters 
         *-----------------------------------------------------------------------------*/
        m_prefix                = m_config->PString("prefix");
        m_path                  = m_config->PString("path");
        m_outputFormat          = m_config->PString("outputFormat");
        m_frequency             = m_config->PInt("frequency");
        m_writeMesh             = m_config->PBool("writeMesh");
        m_writeDrainage         = m_config->PBool("writeDrainage");
        
        if(m_path[m_path.length()-1] != '/') m_path = m_path + "/";

        /*-----------------------------------------------------------------------------
         * Check if path is valid 
         *-----------------------------------------------------------------------------*/
        {
            char tempFileName[1024]={};

            sprintf(tempFileName, "%stemp.txt", m_path.c_str());

            FILE *f = fopen(tempFileName, "w");
            if(f)
            {
                fclose(f);
                remove(tempFileName);
            }
            else
            {
                cerr << "Error: invalid path specified for output-file.." << endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: ~SurfaceTopologyOutput
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    SurfaceTopologyOutput::~SurfaceTopologyOutput()
    {
    }


    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: RegisterScalarField
     * Description:  Method to register scalar-fields for output, which are destroyed 
     *               immediately after they are written.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopologyOutput::RegisterScalarField(ScalarField<float> *sf)
    {
        m_registeredScalarFields.push_back(sf);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: Write
     * Description:  Writes output after specific intervals
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopologyOutput::Write()
    {
        float t = m_model->GetTime();
        int   ts = m_model->GetTimeStep();
        
        if(ts % m_frequency) 
        {
            /*-----------------------------------------------------------------------------
             * Delete registered scalar-fields
             *-----------------------------------------------------------------------------*/
            for(unsigned int i=0; i<m_registeredScalarFields.size(); i++)
            {
                delete m_registeredScalarFields[i];
            }
            m_registeredScalarFields.clear();
            
            return;
        }

        if(m_outputFormat=="vtk")
        {
            if(m_writeMesh)     WriteVTKMesh(t, ts);
            if(m_writeDrainage) WriteVTKDrainage(t, ts);
        }
        else
            cerr << "Warning: Only vtk-output is currently supported." << endl;
    }

    /*-----------------------------------------------------------------------------
     * Private internals 
     *-----------------------------------------------------------------------------*/

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: WriteDataArray
     * Description:  Write a data-array associated with vtk point-data
     *--------------------------------------------------------------------------------------
     */
    template<class T>
    void SurfaceTopologyOutput::WriteDataArray(ofstream &ofs, int nElem, T *data, int nComponents, string dataType, string name)
    {
        const int itemsPerLine = 10;
        char buffer[1024] = {0};
        
        sprintf(buffer, "<DataArray type=\"%s\" Name=\"%s\" format=\"ascii\"  NumberOfComponents=\"%d\">",
                dataType.c_str(), name.c_str(), nComponents);

        ofs << buffer << endl;
        
        for(int i=0; i<nElem; i++)
        {
            ofs << data[i] << " ";
            if((!(i%itemsPerLine)) && (i>0)) ofs << endl;
        }

        ofs << "</DataArray>" << endl;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: WriteVTKMesh
     * Description:  Write unstructured-grid along with associated field variables
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopologyOutput::WriteVTKMesh( float t, int ts)
    {
        const SurfaceTopology *st = m_surfaceTopology;
        int np = st->m_nMeshPoints;
        Triangulator *tr = st->m_triangulator;

        char fileName[256]={0};
        sprintf(fileName, "%s%s.mesh.%d.vtu", m_path.c_str(), m_prefix.c_str(), ts);
        
        ofstream meshFile;
        meshFile.precision(4);
        meshFile.open(fileName, ios_base::out);
        printf ("Writing %s\n", fileName);

        meshFile << "<?xml version=\"1.0\"?>" << endl;
        meshFile << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << endl;
        meshFile << "<UnstructuredGrid>" << endl;
        
        /* piece begin */
        {
            char buffer[1024] = {0};
            sprintf(buffer, "<Piece NumberOfPoints=\"%d\" NumberOfCells=\"%ld\">", 
                    st->m_nMeshPoints, tr->GetNumTriangles());
            meshFile << buffer << endl;
        }

        /* PointData */
        meshFile << "<PointData Scalars=\"h\">" << endl;
        {
            /* h */
            {
                vector<float> h(np); for(int i=0; i<np; i++) h[i] = st->Zp(i)*SCALAR;
                WriteDataArray(meshFile, np, h.data(), 1, "Float32", "h");
            }
            
            /* bc */
            {
                vector<float> bc(np); for(int i=0; i<np; i++) bc[i] = st->B(i);
                WriteDataArray(meshFile, np, bc.data(), 1, "Float32", "bc");
            }
     
            /* cid */
            {
                vector<int> cid(np); for(int i=0; i<np; i++) cid[i] = st->C(i);
                WriteDataArray(meshFile, np, cid.data(), 1, "Int32", "cid");
            }

            /* rid */
            {
                vector<int> rid(np); for(int i=0; i<np; i++) rid[i] = st->R(i);
                WriteDataArray(meshFile, np, rid.data(), 1, "Int32", "rid");
            }

            /* id */
            {
                vector<int> id(np); for(int i=0; i<np; i++) id[i] = i;
                WriteDataArray(meshFile, np, id.data(), 1, "Int32", "id");
            }

            /* dh */
            {
                vector<float> dh(np); for(int i=0; i<np; i++) dh[i] = st->Z(i)*SCALAR - 
                                                                      st->Z0(i)*SCALAR;
                WriteDataArray(meshFile, np, dh.data(), 1, "Float32", "dh");
            }

            /* order */
            {
                vector<int> order(np); for(int i=0; i<np; i++) order[i] = st->O(i);
                WriteDataArray(meshFile, np, order.data(), 1, "Int32", "order");
            }

            /* t */
            {
                vector<float> mt(np); for(int i=0; i<np; i++) mt[i] = t;
                WriteDataArray(meshFile, np, mt.data(), 1, "Float32", "t");
            }


            /*-----------------------------------------------------------------------------
             * Output registered scalar-fields
             *-----------------------------------------------------------------------------*/
            for(unsigned int i=0; i<m_registeredScalarFields.size(); i++)
            {
                ScalarField<float>* sf = m_registeredScalarFields[i];
                int length = sf->GetLength();

                vector<float> vf(length); 
                for(int j=0; j<length; j++) vf[j] = (*sf)(j);
                
                WriteDataArray(meshFile, length, vf.data(), 1, "Float32", sf->GetName());
            }

            /*-----------------------------------------------------------------------------
             * Delete registered scalar-fields
             *-----------------------------------------------------------------------------*/
            for(unsigned int i=0; i<m_registeredScalarFields.size(); i++)
            {
                delete m_registeredScalarFields[i];
            }
            m_registeredScalarFields.clear();
        }
        meshFile << "</PointData>" << endl;

        /* CellData */
        meshFile << "<CellData>" << endl;
        meshFile << "</CellData>" << endl;

        /* Points */
        meshFile << "<Points>" << endl;
        {
            vector<float> coords(np*3);
            for(int i=0; i<np; i++)
            {
                coords[i*3]   = st->X(i);
                coords[i*3+1] = st->Y(i);
                coords[i*3+2] = st->Z(i)*SCALAR;
            }

            WriteDataArray(meshFile, np*3, coords.data(), 3, "Float32", "Points");
        }
        meshFile << "</Points>" << endl;

        /* Cells */
        meshFile << "<Cells>" << endl;
        {
            int ntri = tr->GetNumTriangles();
            unsigned int **triIndices = tr->GetTriangleIndices();
            
            /* connectivity */
            {
                vector<long int> triIndicesVector(ntri*3);

                for(int i=0; i<ntri; i++)
                    for(int j=0; j<3; j++)
                        triIndicesVector[i*3+j] = triIndices[i][j];
                
                WriteDataArray(meshFile, ntri*3, triIndicesVector.data(), 1, "Int64", "connectivity");
            }

            /* offsets */
            {
                vector<long int> offsets(ntri);
                
                for(int i=0; i<ntri; i++) offsets[i] = (i+1)*3;
                WriteDataArray(meshFile, ntri, offsets.data(), 1, "Int64", "offsets");
            }
            
            /* types */
            {
                vector<int> types(ntri);

                for(int i=0; i<ntri; i++) types[i] = 5;
                WriteDataArray(meshFile, ntri, types.data(), 1, "UInt8", "types");
            }
        }
        meshFile << "</Cells>" << endl;
        meshFile << "</Piece>" << endl;
        meshFile << "</UnstructuredGrid>" << endl;
        meshFile << "</VTKFile>" << endl;

        meshFile.close();
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: WriteVTKDrainage
     * Description:  Write vtp file for drainage-network.
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopologyOutput::WriteVTKDrainage( float t, int ts)
    {
        struct Recursor
        {
            static void TraverseUpstream(const SurfaceTopology *st, int node, vector<int> *pointVec)
            {
                for(int i=0; i<st->Dn(node); i++)
                {
                    int donor = st->D(node)[i];

                    TraverseUpstream(st, donor, pointVec);
                }
                pointVec->push_back(node);
            }
        };

        vector<int> globalPointIdList;
        vector<int> connectivity;
        vector<int> offsets;
        int polyLineCount = 0;

        for(int i=0; i<m_surfaceTopology->m_nMeshPoints; i++)
        {
            vector<int> catchmentNodes;
            vector<int> visited(m_surfaceTopology->m_nMeshPoints);
            if(m_surfaceTopology->B(i) || (m_surfaceTopology->R(i)==i)) 
            {
                if(m_surfaceTopology->Dn(i))
                {
                    Recursor::TraverseUpstream(m_surfaceTopology, i, 
                                               &catchmentNodes);
                }
                
                if(catchmentNodes.size())
                {
                    for(unsigned int j=0; j<catchmentNodes.size(); j++)
                    {
                        int node = catchmentNodes[j];

                        vector<int> pointIdList; 
                        int visitedAddCount=0;
                        while(!visitedAddCount)
                        {
                            if(visited[node]) visitedAddCount++;
                            pointIdList.push_back(node);
                            visited[node] = 1;
                            
                            node = m_surfaceTopology->R(node);
                            
                            /* Termination condition */
                            if(m_surfaceTopology->R(node)==node)
                            {
                                int lastnode = m_surfaceTopology->R(node);
                                if(visited[lastnode]) visitedAddCount++;
                                
                                pointIdList.push_back(lastnode);
                                visited[lastnode] = 1;
                                break;
                            }
                        }

                        if(pointIdList.size()>1)
                        {
                            int pointIdListSize = pointIdList.size();
                            int connectivitySize = connectivity.size();
                            int offsetsSize = offsets.size();

                            for(int i=0; i<pointIdListSize; i++)
                            {
                                globalPointIdList.push_back(pointIdList[i]);
                                connectivity.push_back(connectivitySize+i);
                            }
                            
                            if(offsetsSize)
                                offsets.push_back(offsets[offsetsSize-1] + pointIdListSize);
                            else
                                offsets.push_back(pointIdListSize);

                            polyLineCount++;
                        }
                    }
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Write drainage network
         *-----------------------------------------------------------------------------*/
        int globalPointIdListSize = globalPointIdList.size();
        char fileName[256]={0};
        sprintf(fileName, "%s%s.drainage.%d.vtp", m_path.c_str(), m_prefix.c_str(), ts);
        printf ("Writing %s\n", fileName);
        
        ofstream drainageFile;
        drainageFile.precision(4);
        drainageFile.open(fileName, ios_base::out);

        drainageFile << "<?xml version=\"1.0\"?>" << endl;
        drainageFile << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\">" << endl;
        drainageFile << "<PolyData>" << endl;
        
        /* piece begin */
        {
            char buffer[1024] = {0};
            sprintf(buffer, "<Piece NumberOfPoints=\"%ld\" NumberOfVerts=\"0\" NumberOfLines=\"%d\" NumberOfStrips=\"0\" NumberOfPolys=\"0\">", 
                    (long int)(globalPointIdList.size()), polyLineCount);
            drainageFile << buffer << endl;
        }

        /* Points */
        drainageFile << "<Points>" << endl;
        {
            vector<float> coords(globalPointIdListSize*3);
            for(int i=0; i<globalPointIdListSize; i++)
            {
                coords[i*3]   = m_surfaceTopology->X(globalPointIdList[i]);
                coords[i*3+1] = m_surfaceTopology->Y(globalPointIdList[i]);
                coords[i*3+2] = m_surfaceTopology->Z(globalPointIdList[i])*SCALAR;
            }

            WriteDataArray(drainageFile, globalPointIdListSize*3, coords.data(), 3, "Float32", "Points");
        }
        drainageFile << "</Points>" << endl;

        /* CellData */
        drainageFile << "<PointData>" << endl;
            /* h */
            {
                vector<float> h(globalPointIdListSize); 
                for(int i=0; i<globalPointIdListSize; i++) h[i] = m_surfaceTopology->Zp(globalPointIdList[i])*SCALAR;
                WriteDataArray(drainageFile, globalPointIdListSize, h.data(), 1, "Float32", "h");
            }
            
            /* cid */
            {
                vector<int> cid(globalPointIdListSize); 
                for(int i=0; i<globalPointIdListSize; i++) cid[i] = m_surfaceTopology->C(globalPointIdList[i]);
                WriteDataArray(drainageFile, globalPointIdListSize, cid.data(), 1, "Int32", "cid");
            }

            /* rid */
            {
                vector<int> rid(globalPointIdListSize); 
                for(int i=0; i<globalPointIdListSize; i++) rid[i] = m_surfaceTopology->R(globalPointIdList[i]);
                WriteDataArray(drainageFile, globalPointIdListSize, rid.data(), 1, "Int32", "rid");
            }

            /* id */
            {
                vector<int> id(globalPointIdListSize); 
                for(int i=0; i<globalPointIdListSize; i++) id[i] = globalPointIdList[i];
                WriteDataArray(drainageFile, globalPointIdListSize, id.data(), 1, "Int32", "id");
            }

            /* dh */
            {
                vector<float> dh(globalPointIdListSize); 
                for(int i=0; i<globalPointIdListSize; i++) 
                    dh[i] = m_surfaceTopology->Z(globalPointIdList[i])*SCALAR - 
                            m_surfaceTopology->Zp(globalPointIdList[i])*SCALAR;
                WriteDataArray(drainageFile, globalPointIdListSize, dh.data(), 1, "Float32", "dh");
            }        
        drainageFile << "</PointData>" << endl;

        /* CellData */
        drainageFile << "<CellData>" << endl;
        drainageFile << "</CellData>" << endl;
        
        /* Lines */
        drainageFile << "<Lines>" << endl;
        {
            WriteDataArray(drainageFile, connectivity.size(), connectivity.data(), 1, "Int64", "connectivity");
            WriteDataArray(drainageFile, offsets.size(), offsets.data(), 1, "Int64", "offsets");
        }
        drainageFile << "</Lines>" << endl;


        drainageFile << "</Piece>" << endl;
        drainageFile << "</PolyData>" << endl;
        drainageFile << "</VTKFile>" << endl;

        drainageFile.close();            
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  SurfaceTopologyOutput
     *      Method:  SurfaceTopologyOutput :: WriteTXT
     * Description:  Yet to be implemented
     *--------------------------------------------------------------------------------------
     */
    void SurfaceTopologyOutput::WriteTXT( float t, int ts)
    {
    }
}}

