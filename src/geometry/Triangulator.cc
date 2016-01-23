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
 *       Filename:  Triangulator.cc
 *
 *    Description:  Implementation of the Triangulator interface.
 *
 *        Version:  1.0
 *        Created:  20/02/14 11:43:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)                  
 *
 * =====================================================================================
 */
#include <Triangulator.hh>
#include <iostream>
#include <limits>
#include <math.h>
#include <new>
#include <algorithm>

namespace src{ namespace geometry {

double const Triangulator::PI = 3.1415926535897932384626;
using namespace std;

/* #####  PUBLIC INTERFACE  ############################### */
/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator
 * Description:  The constructor takes 3 paramers:
 *               1. ns, the number of sites (nodes) in the triangulation.
 *               2. s is a 2D float-array where only the first two elements in the 
 *                  fastest moving direction are used - the first element is expected to
 *                  be the x-coordinate and the second element that of y. 
 *               3. attr is a bit-masked field that encodes what attributes are to be
 *                  computed. See the header file for more details and the associated 
 *                  test-file demonstrates sample usage.
 *--------------------------------------------------------------------------------------
 */
Triangulator::Triangulator(int ns, float **s, unsigned int attr)
:m_attributes(attr),
m_nSites(ns),
m_nInputSites(ns)
{
    /* Allocate storage */
    AllocateStorage();

    /* Assign geometry */
    for( int i=0; i<m_nSites; i++ )
    {
        if( i < m_nInputSites ) 
        {
            m_sites[i].m_coord = &(s[i]);
        }
        else
        {
            /* Setting super-triangle coordinates to the first site
             * before it is computed */
            m_superTriangle[i%3][0] = s[0][0];
            m_superTriangle[i%3][1] = s[0][1];
            
            m_sites[i].m_coord = &(m_superTriangle[i%3]);
        }
        
        m_sites[i].m_id = i;
    }
    
    /* Initialize super-triangle if required */
    if(m_attributes & Triangulator_SuperTriangle) InitSuperTriangle();  

    /* Sort sites */
    SortSites();

    /* Recursive call to compute triangulation */
    Delaunay(0, m_nSites, &(m_le), &(m_re));

    /* Edge accounting */
    m_nEdges = m_nSites * 3 - m_dEdgePool->GetNumFree();
    m_nFaces = m_nEdges - m_nSites + 2;
    m_nTriangles = 0;

    /* Mark hull-nodes */
    MarkHull();

    /* Generate triangle-indices */
    if( m_attributes & Triangulator_TriangleIndices ) GenerateTriangleIndices();
    
    /* Generate voronoi vertices*/
    if( m_attributes & Triangulator_VoronoiVertices ) 
    {
        GenerateVoronoiVertices();
        
        VSite *ptr = (VSite*)m_vEdgePool->GetDataPointer();
        if(ptr)
		    m_nVoronoiVertices = m_nSites * 2 - m_vEdgePool->GetNumFree();
        else
            m_nVoronoiVertices = 0;
    }

    /* Generate node-neighbours and compute voronoi-related attributes */
    GenerateNodeNeighbours();
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  ~Triangulator
 * Description:  Destructor.
 *--------------------------------------------------------------------------------------
 */
Triangulator::~Triangulator()
{
    if(m_superTriangle)
    {
        delete [] m_superTriangle[0];
        delete [] m_superTriangle;
    }

    if( m_sites )           delete [] m_sites;
    if( m_dEdgePool )       delete m_dEdgePool;
    if( m_vEdgePool )       delete m_vEdgePool;    
    if( m_hull )            delete [] m_hull;    
    if( m_outputHull )      delete [] m_outputHull;    
    
    if( m_tIndices )
    {
        delete [] m_tIndices[0];
        delete [] m_tIndices;
    }

    if( m_tNeighbours )
    {
        delete [] m_tNeighbours[0];
        delete [] m_tNeighbours;
    }

    if( m_nNeighbours ) delete [] m_nNeighbours;
    
    if( m_neighbours )
    {
        delete [] m_neighbours[0];
        delete [] m_neighbours;
    }

    if( m_voronoiSides )
    {
        delete [] m_voronoiSides[0];
        delete [] m_voronoiSides;
    }

    if( m_voronoiArea ) delete [] m_voronoiArea;    
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetTriangleIndices
 * Description:  Returns the indices of nodes that form the triangulation. Note, the 
 *               indices are in terms of the original ordering of the input nodes. Note, 
 *               the Triagulator class maintains ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
unsigned int **Triangulator::GetTriangleIndices()
{
	return m_tIndices;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetVoronoiSides
 * Description:  Returns the length of the sides of voronoi cells. Note, the Triagulator
 *               class maintains ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
float **Triangulator::GetVoronoiSides( )
{
	return m_voronoiSides;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetVoronoiCellAreas
 * Description:  Returns the voronoi cell areas. Note, the Triagulator class maintains 
 *               ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
float *Triangulator::GetVoronoiCellAreas( )
{
	return m_voronoiArea;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetNumNeighbours
 * Description:  Returns an array containing the number of natural neighbours for each
 *               node. Note, the Triagulator class maintains ownership of the array
 *               returned.
 *--------------------------------------------------------------------------------------
 */
unsigned int *Triangulator::GetNumNeighbours( )
{
	return m_nNeighbours;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetNeighbours
 * Description:  Returns an array containing the natural neighbour-indices for each 
 *               node. Note, the Triagulator class maintains ownership of the array
 *               returned.
 *--------------------------------------------------------------------------------------
 */
unsigned int **Triangulator::GetNeighbours( )
{
	return m_neighbours;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetHull
 * Description:  Returns an integer array with the convex-hull nodes marked. Note, the 
 *               Triagulator class maintains ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
int *Triangulator::GetHull( )
{
	return m_outputHull;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetNumTriangles
 * Description:  Returns the number of triangles, excluding the super-triangle, if 
 *               specified.
 *--------------------------------------------------------------------------------------
 */
long int Triangulator::GetNumTriangles()
{
    return m_nTriangles;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetNumFaces
 * Description:  Returns the number of faces in the triangulation.
 *--------------------------------------------------------------------------------------
 */
long int Triangulator::GetNumFaces()
{
    return m_nFaces;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetNumVoronoiVertices
 * Description:  Returns the number of voronoi vertices in the voronoi diagram. Note, 
 *               the Triagulator class maintains ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
long int Triangulator::GetNumVoronoiVertices()
{
    if (m_vEdgePool)
    {
        VSite *ptr = (VSite*)m_vEdgePool->GetDataPointer();

        if(ptr)
        {
            return m_nVoronoiVertices;
        }
    }

    return 0;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GetVoronoiVertices
 * Description:  Returns the voronoi vertices. Note, the Triagulator class maintains 
 *               ownership of the array returned.
 *--------------------------------------------------------------------------------------
 */
VSite *Triangulator::GetVoronoiVertices()
{
    if (m_vEdgePool)
    {
        VSite *ptr = (VSite*)m_vEdgePool->GetDataPointer();

        if(ptr)
        {
            int numFree = m_vEdgePool->GetNumFree();

            return ptr+numFree;
        }
    }

    return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  operator<<
 *  Description:  Prints some basic attributes of a triangulation.
 * =====================================================================================
 */
std::ostream& operator<<(std::ostream& os, const Triangulator& t)
{
	cout << "Triangulator (" << &(t) << ")" << endl;
	
	cout << "\tNum Sites: " << t.m_nSites << endl;
	cout << "\tNum Edges: " << t.m_nEdges << endl;
	cout << "\tNum Triangles: " << t.m_nTriangles << endl;
    if(t.m_attributes & Triangulator::Triangulator_VoronoiVertices)
    	cout << "\tNum Voronoi Vertices: " << t.m_nVoronoiVertices << endl;

    return os;
}


/* #####   LOCAL MEMBER FUNCTIONS    ############################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: ComputeBound
 * Description:  Computes the upper and lower coordinate bounds of the input points.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::ComputeBound( float *minx, float *miny, float *maxx, float *maxy )
{
    *maxx = -numeric_limits<float>::max();
    *maxy = -numeric_limits<float>::max();
    
    *minx = numeric_limits<float>::max();
    *miny = numeric_limits<float>::max();

    for( int i=0; i<m_nSites; i++ )
    {
        float *coord = *(m_sites[i].m_coord);

        if( *maxx < coord[0] ) *maxx = coord[0];
        if( *maxy < coord[1] ) *maxy = coord[1];
        if( *minx > coord[0] ) *minx = coord[0];
        if( *miny > coord[1] ) *miny = coord[1];
    }
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: InitSuperTriangle
 * Description:  Initialize the super-triangle that contains all the input points.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::InitSuperTriangle()
{
    float maxx, minx, maxy, miny;
    float centrex, centrey;
    float radius;
    
    centrex = 0; centrey = 0; 
    ComputeBound( &minx, &miny, &maxx, &maxy );
    radius = sqrt((maxx - minx) * (maxx - minx) + 
                  (maxy - miny) * (maxy - miny));
            
    centrex = minx + (maxx - minx) / 2.0f;
    centrey = miny + (maxy - miny) / 2.0f;

    m_superTriangle[0][0] = centrex - tan(PI/3.0f)*radius;
    m_superTriangle[0][1] = centrey - radius;

    m_superTriangle[1][0] = centrex + tan(PI/3.0f)*radius;
    m_superTriangle[1][1] = centrey - radius;
            
    m_superTriangle[2][0] = centrex;
    m_superTriangle[2][1] = centrey + radius/cos(PI/3.0f);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: AllocateStorage
 * Description:  Allocates storage for some of the attributes that are to be computed.
 *               Note, some memory allocations are done at the individual 
 *               function-level.
 *
 *               Note, attribute m_nSites is incremented by 3 if the input nodes are to
 *               be contained by the super-triangle.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::AllocateStorage()
{
    m_superTriangle = NULL;
    /* Adjust numSites if required */
    if(m_attributes & Triangulator_SuperTriangle) 
    {
        m_nSites += 3;

        /* Allocate memory for super-triangle */
        m_superTriangle     = new float*[3];
        m_superTriangle[0]  = new float[6];
        for(int i=0; i<3; i++) m_superTriangle[i] = m_superTriangle[0]+i*2;        
    }

    m_sites                 = new Site[m_nSites];
    m_dEdgePool             = new MemoryPool( sizeof(QuadEdge), m_nSites * 3, MemoryPool::Fixed );

    /* Allocate memory-pools */
    if(m_attributes & Triangulator_VoronoiVertices)
        m_vEdgePool         = new MemoryPool( sizeof(VSite), m_nSites * 2, MemoryPool::Fixed );
    else
        m_vEdgePool         = NULL;

    /* Allocate memory for hull */
    m_hull = new int[m_nSites];
    m_outputHull = new int[m_nSites];

    /* Set triangle related pointers to NULL.
     * They are allocated after triangulation 
     * and only if needed. */
    m_tIndices              = NULL;
    m_tNeighbours           = NULL;

    /* Set voronoi related pointers to NULL.
     * They are allocated after triangulation
     * and only if needed. */
    m_nNeighbours           = NULL;
    m_neighbours            = NULL;
    m_voronoiSides          = NULL;
    m_voronoiArea           = NULL;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: Delaunay
 * Description:  This function implements the divide and conquer algorithm outlined in 
 *               Guibas and Stolfi (1985). This is a direct line-by-line translation of 
 *               the algorithm in Guibas and Stolfi (1985), p. 114
 *--------------------------------------------------------------------------------------
 */
void Triangulator::Delaunay( int sl, int sh, Edge **le, Edge **re )
{
    Site *sites = m_sites;
    
    if (sh == sl+2) 
    {
        Edge *a = MakeEdge();
        a->Org_Set(&sites[sl]); a->Dest_Set(&sites[sl+1]);
        *le = a; *re = a->Sym();
    }
    else if (sh == sl+3) 
    {
        Edge *a = MakeEdge();
        Edge *b = MakeEdge();
        double ct;
        Site::CCW(&(sites[sl]), &(sites[sl+1]), &(sites[sl+2]), &ct);
        Splice(a->Sym(), b);
        
        a->Org_Set(&sites[sl]);   a->Dest_Set(&sites[sl+1]);
        b->Org_Set(&sites[sl+1]); b->Dest_Set(&sites[sl+2]);
        
        if (ct == 0.0) 
        { 
            *le = a; *re = b->Sym(); 
        }
        else
        { 
            Edge *c = Connect(b, a);
            if (ct > 0.0) 
            { 
                *le = a; *re = b->Sym(); 
            }
            else 
            { 
                *le = c->Sym(); *re = c; 
            }
        }
    }
    else
    {
        Edge *ldo = 0, *ldi = 0, *rdi = 0, *rdo = 0;
        Edge *basel = 0, *lcand = 0, *rcand = 0;

        int sm = (sl+sh)/2;

        Delaunay ( sl, sm, &ldo, &ldi );
        Delaunay ( sm, sh, &rdi, &rdo);

        while (1) 
        {
            if (LeftOf(rdi->Org(), ldi)) ldi = ldi->Lnext();
            else if (RightOf(ldi->Org(), rdi)) rdi = rdi->Sym()->Onext();
            else break;
        }

        basel = Connect(rdi->Sym(), ldi);
        if (ldi->Org() == ldo->Org()) ldo = basel->Sym();
        if (rdi->Org() == rdo->Org()) rdo = basel;

        while (1) 
        {

            lcand = basel->Sym()->Onext();
            if (RightOf(lcand->Dest(), basel))
            {
                while (Site::InCircle(basel->Dest(), basel->Org(), lcand->Dest(), lcand->Onext()->Dest()))
                {
                    Edge *t = lcand->Onext();
                    
                    Delete(lcand);
                    lcand = t;
                }
            }

            rcand = basel->Oprev();
            if (RightOf(rcand->Dest(), basel))
            {
                while (Site::InCircle(basel->Dest(), basel->Org(), rcand->Dest(), rcand->Oprev()->Dest())) 
                {
                    Edge *t = rcand->Oprev();

                    Delete(rcand);
                    rcand = t;
                }
            }

            if (!RightOf(lcand->Dest(), basel) && !RightOf(rcand->Dest(), basel)) break;

            if ( !RightOf(lcand->Dest(), basel) ||
                ( RightOf(rcand->Dest(), basel) && 
                Site::InCircle(lcand->Dest(), lcand->Org(), rcand->Org(), rcand->Dest())))
                basel = Connect(rcand, basel->Sym());
            else
                basel = Connect(basel->Sym(), lcand->Sym());
        }
        *le = ldo; *re = rdo;
    }
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: MarkHull
 * Description:  Creates a convex hull of the input nodes - note that the 
 *               super-triangular nodes are ignored, if present.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::MarkHull()
{
    /*-----------------------------------------------------------------------------
     * Note, we keep two sets of hulls. When a super-triangle is used, we 'peel' 
     * off the super-triangular edges to mark the next layer of nodes as the actual
     * hull nodes.
     *-----------------------------------------------------------------------------*/
    memset( m_hull, 0, sizeof( int ) * m_nSites );
    memset( m_outputHull, 0, sizeof( int ) * m_nSites );

    /*-----------------------------------------------------------------------------
     * Mark hull nodes. 
     *-----------------------------------------------------------------------------*/
    Edge *start = 0, *le = 0;
    
    start = le = m_le;        
    do
    {
        m_hull[le->Org()->m_id]   = 1;
        m_hull[le->Dest()->m_id]  = 1;
        le = le->Rprev();
    }while(le != start);

    if(m_attributes & Triangulator_SuperTriangle)
    {
        Edge *start = 0, *le = 0;
        
        start = le = m_le;
        
        memset( m_outputHull, 0, sizeof( int ) * m_nSites );
        do
        {
            Edge *leDnext = 0, *leDnextStart = 0;
            leDnextStart = leDnext = le->Dnext();
            do
            {
                if(leDnext->Org()->m_id < m_nInputSites) 
                    m_outputHull[leDnext->Org()->m_id] = 1;

                leDnext = leDnext->Dnext();
            }while(leDnext != leDnextStart);

            le = le->Rprev();
        }while(le != start);
    }
    else
    /*-----------------------------------------------------------------------------
     * When a super-triangle is not used m_hull and m_outputHull are the same.
     *-----------------------------------------------------------------------------*/
    {
        memcpy(m_outputHull, m_hull, sizeof(int)*m_nSites);
    }    
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GenerateTriangleIndices
 * Description:  Generates triangle indices for the triangulation.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::GenerateTriangleIndices()
{
    int **edgeToTriangle = NULL;
    
    /* Allocate memory for triangle indices */
    m_tIndices = new unsigned int*[m_nFaces];
    m_tIndices[0] = new unsigned int[m_nFaces * 3];
    memset( m_tIndices[0] , 0, sizeof(unsigned int) * m_nFaces * 3 );
    
    if( m_attributes & Triangulator_TriangleNeighbours )
    {
        m_tNeighbours = new unsigned int*[m_nFaces];
        m_tNeighbours[0] = new unsigned int[m_nFaces * 3];

        edgeToTriangle = new int*[m_nSites * 3];
        edgeToTriangle[0] = new int[m_nSites * 3 * 2];
    }
    
    for( int i=0; i<m_nFaces; i++ )
    {
        m_tIndices[i] = m_tIndices[0]+i*3;

        if( m_attributes & Triangulator_TriangleNeighbours )
        {
            m_tNeighbours[i] = m_tNeighbours[0]+i*3;
            
            m_tNeighbours[i][0]=m_tNeighbours[i][1]=m_tNeighbours[i][2]=m_nFaces-1;
        }
    }

    if( m_attributes & Triangulator_TriangleNeighbours )
    {
        for( int i=0; i<m_nSites * 3; i++ )
        {
            edgeToTriangle[i] = edgeToTriangle[0]+i*2;
            
            edgeToTriangle[i][0]=edgeToTriangle[i][1]=m_nFaces-1;
        }
    }

    /*-----------------------------------------------------------------------------
     * Loop over all edges and harvest triangle-indices 
     *-----------------------------------------------------------------------------*/
    QuadEdge *qedges = (QuadEdge*)m_dEdgePool->GetDataPointer();
    int maxEdges = m_nSites * 3;
     
    /* Reset visit-count */ 
    for (int i = 0; i < maxEdges; i++) qedges[i].ResetVisited();

    int tCount = 0;
    Edge *e = 0, *eStart = 0, *eOnext = 0, *eLnext = 0;
    for (int i = 0; i < maxEdges; i++) 
    {
        e = eStart = qedges[i].m_e;
        
        if( e->Qedge()->IsFree() ) continue;
        do
        {
            eOnext = e->Onext();
            eLnext = e->Lnext();
            
            if( (e->Qedge()->Visited()<2) && 
                (eLnext->Qedge()->Visited()<2) && 
                (eOnext->Qedge()->Visited()<2) )
            {
                if( (eLnext->Org() == e->Dest()) && (eLnext->Dest() == eOnext->Dest()) )
                {
                    /* Ignore nodes introduced internally */
                    if( m_attributes & Triangulator_SuperTriangle )
                    {
                        if( (!m_hull[e->Org()->m_id]) &&
                            (!m_hull[e->Dest()->m_id]) && 
                            (!m_hull[eOnext->Dest()->m_id]) )
                        {
                            m_tIndices[tCount][0] = e->Org()->m_id;
                            m_tIndices[tCount][1] = e->Dest()->m_id;
                            m_tIndices[tCount][2] = eOnext->Dest()->m_id;

                            if( m_attributes & Triangulator_TriangleNeighbours )
                            {
                                edgeToTriangle[e->Qedge() - qedges][e->Qedge()->Visited()] = tCount;
                                edgeToTriangle[eOnext->Qedge() - qedges][eOnext->Qedge()->Visited()] = tCount;
                                edgeToTriangle[eLnext->Qedge() - qedges][eLnext->Qedge()->Visited()] = tCount;
                            }
                            tCount++;
                        }
                    }
                    else
                    {
                        m_tIndices[tCount][0] = e->Org()->m_id;
                        m_tIndices[tCount][1] = e->Dest()->m_id;
                        m_tIndices[tCount][2] = eOnext->Dest()->m_id;
                        
                        if( m_attributes & Triangulator_TriangleNeighbours )
                        {
                            edgeToTriangle[e->Qedge() - qedges][e->Qedge()->Visited()] = tCount;
                            edgeToTriangle[eOnext->Qedge() - qedges][eOnext->Qedge()->Visited()] = tCount;
                            edgeToTriangle[eLnext->Qedge() - qedges][eLnext->Qedge()->Visited()] = tCount;
                        }
                        tCount++;
                    }

                    e->Qedge()->IncrementVisited();
                    eOnext->Qedge()->IncrementVisited();
                    eLnext->Qedge()->IncrementVisited();
                }
            }
            e = eOnext;
        }while( e != eStart );
    }
    
    m_nTriangles = tCount;

    /*-----------------------------------------------------------------------------
     * Generate triangle-neighbour indices if necessary  
     *-----------------------------------------------------------------------------*/
    if( m_attributes & Triangulator_TriangleNeighbours )
    {
        int *tNeighbourCount = NULL;

        tNeighbourCount = new int[m_nFaces];
        memset( tNeighbourCount, 0, sizeof( int ) * m_nFaces );
        
        for( int i=0; i<maxEdges; i++ )
        {
            if( qedges[i].IsFree() ) continue;
        
            if( edgeToTriangle[i][0] != (m_nFaces-1) )
                m_tNeighbours[edgeToTriangle[i][0]][tNeighbourCount[edgeToTriangle[i][0]]++] = edgeToTriangle[i][1];
        
            if( edgeToTriangle[i][1] != (m_nFaces-1) )
                m_tNeighbours[edgeToTriangle[i][1]][tNeighbourCount[edgeToTriangle[i][1]]++] = edgeToTriangle[i][0];
        }
        
        delete [] edgeToTriangle[0];
        delete [] edgeToTriangle;
        delete [] tNeighbourCount;
    }
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GenerateVoronoiVertices
 * Description:  Generate voronoi vertices.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::GenerateVoronoiVertices()
{
    QuadEdge *qedges = (QuadEdge*)m_dEdgePool->GetDataPointer();
    int maxEdges = m_nSites * 3;
    
    /* Reset visit-count */
    for (int i = 0; i < maxEdges; i++) qedges[i].ResetVisited();

    Edge *e = 0, *eStart = 0, *eOnext = 0, *eLnext = 0;
    for (int i = 0; i < maxEdges; i++)
    {
        e = eStart = qedges[i].m_e;
        
        if( e->Qedge()->IsFree() ) continue;
        
        do
        {
            eOnext = e->Onext();
            eLnext = e->Lnext();
            
            if( (e->Qedge()->Visited()<2) && 
                (eLnext->Qedge()->Visited()<2) && 
                (eOnext->Qedge()->Visited()<2) )
            {
                if( (eLnext->Org() == e->Dest()) && (eLnext->Dest() == eOnext->Dest()) )
                {
                    VSite *vs = (VSite*)m_vEdgePool->NewObject();
                
                    /* Compute circumcenter */
                    Site::Circumcenter( e->Org(), eOnext->Dest(), e->Dest(), vs );
                    
                    /* Attach vornoi site to relevant edges */
                    e->VDest_Set(vs);
                    eOnext->VOrg_Set(vs);
                    eLnext->VDest_Set(vs);
                    
                    /* Increment visit-count */
                    e->Qedge()->IncrementVisited();
                    eOnext->Qedge()->IncrementVisited();
                    eLnext->Qedge()->IncrementVisited();
                }
            }
            e = eOnext;
        }while( e != eStart );
    }
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: GenerateNodeNeighbours
 * Description:  Natural neighbours, length of voronoi-sides of voronoi cells and 
 *               voronoi cell areas are computed in this function.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::GenerateNodeNeighbours()
{
    int maxEdges = m_nSites*3;
    QuadEdge *qedges = (QuadEdge*)m_dEdgePool->GetDataPointer();
    
    /* Reset visit-count */
    for (int i = 0; i < maxEdges; i++) qedges[i].ResetVisited();

    m_nNeighbours = new unsigned int [m_nSites];
    memset( m_nNeighbours, 0, sizeof( unsigned int ) * m_nSites );

    /* Find number of neighbours each node has */
    int nNeighboursSum = 0;
    for( int i=0; i<maxEdges; i++ )
    {
        Edge *e = qedges[i].m_e;
        Site *src = 0, *dst = 0;

        if( e->Qedge()->IsFree() ) continue;
        
        if( e->Qedge()->Visited() == 0 )
        {
            src = e->Org();
            dst = e->Dest();

            if( (src->m_id < m_nInputSites) &&
                (dst->m_id < m_nInputSites) )
            {
                m_nNeighbours[src->m_id]++;
                m_nNeighbours[dst->m_id]++;
                nNeighboursSum += 2;
            }
            e->Qedge()->IncrementVisited();
        }
    }

    /* Allocate memory for voronoi attributes */
    if( m_attributes & (Triangulator_VoronoiCellAreas | Triangulator_VoronoiVertices) )
    {
        m_voronoiArea = new float[m_nSites];
        memset( m_voronoiArea, 0, sizeof( float ) * m_nSites );
    }
    
    if( m_attributes & Triangulator_NodeNeighbours )
    {
        m_neighbours = new unsigned int* [m_nSites];
        m_neighbours[0] = new unsigned int [nNeighboursSum];
        memset( m_neighbours[0], 0, sizeof( unsigned int ) * nNeighboursSum );
    }
    
    if( m_attributes & (Triangulator_VoronoiSides | Triangulator_VoronoiVertices) )
    {
        m_voronoiSides = new float* [m_nSites];
        m_voronoiSides[0] = new float [nNeighboursSum];
        memset( m_voronoiSides[0], 0, sizeof( float ) * nNeighboursSum );
    }

    /* Set array strides */
    int cp = 0;
    for( int i=0; i<m_nSites; i++ )
    {
        if( m_neighbours ) m_neighbours[i] = m_neighbours[0] + cp;
        if( m_voronoiSides ) m_voronoiSides[i] = m_voronoiSides[0] + cp;
        
        cp += m_nNeighbours[i];
    }
    
    /* Temporary work array */
    int *nNeighboursTemp = new int [m_nSites];
    memcpy( nNeighboursTemp, m_nNeighbours, sizeof( int ) * m_nSites );
    
    /*-----------------------------------------------------------------------------
     * Compute lengths of voronoi sides, area of voronoi cells and harvest 
     * natural neighbours.
     *-----------------------------------------------------------------------------*/
    for( int i=0; i<maxEdges; i++ )
    {
        Edge *e = qedges[i].m_e;
        
        if( e->Qedge()->IsFree() ) continue;
        
        Site *src = e->Org();
        Site *dst = e->Dest();
        
        int srcCount = nNeighboursTemp[src->m_id];
        int dstCount = nNeighboursTemp[dst->m_id];
        
        if( e->Qedge()->Visited() == 1 )
        {
            if( (src->m_id < m_nInputSites) &&
                (dst->m_id < m_nInputSites) )
            {
                if( m_attributes & Triangulator_VoronoiVertices )
                {
                    VSite *vsrc = e->VOrg();
                    VSite *vdst = e->VDest();
                
                    if( vsrc && vdst )
                    {
                        /* voronoi sides */
                        if( m_attributes & Triangulator_VoronoiSides )
                        {
                            float diffx = ( vsrc->m_coord[0] - vdst->m_coord[0] );
                            float diffy = ( vsrc->m_coord[1] - vdst->m_coord[1] );
                            float dist = sqrt( diffx*diffx + diffy*diffy );

                            m_voronoiSides[src->m_id][--srcCount] = dist;
                            m_voronoiSides[dst->m_id][--dstCount] = dist;
                        }
                    
                        if( m_attributes & Triangulator_VoronoiCellAreas )
                        {
                            /* voronoi-cell area for each voronoi vertex in bounded
                             * except for the hull nodes which have an infinite area */
                            
                            double vCellArea = 0;
                            if( !m_hull[src->m_id] )
                            {
                                Site::CCW( src, vsrc, vdst, &vCellArea );
                                m_voronoiArea[src->m_id] += FABS( vCellArea ) * 0.5;
                            }
                            else
                            {
                                m_voronoiArea[src->m_id] = numeric_limits<float>::max();
                            }
                        
                            if( !m_hull[dst->m_id] )
                            {
                                Site::CCW( dst, vsrc, vdst, &vCellArea );
                                m_voronoiArea[dst->m_id] += FABS( vCellArea ) * 0.5;
                            }
                            else
                            {
                                m_voronoiArea[dst->m_id] = numeric_limits<float>::max();
                            }
                        }
                    }
                }

                /* Storing the actual neighbours of each node */
                if( m_attributes & Triangulator_NodeNeighbours )
                {
                    m_neighbours[src->m_id][--nNeighboursTemp[src->m_id]] = dst->m_id;
                    m_neighbours[dst->m_id][--nNeighboursTemp[dst->m_id]] = src->m_id;
                }
            }
            e->Qedge()->IncrementVisited();
        }
    }
    delete [] nNeighboursTemp;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: SortSites
 * Description:  The functor defined in Topology.hh is used to sort the input nodes.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::SortSites()
{
    sort(m_sites, m_sites+m_nSites, SiteComparator());
}


/* #####   TOPOLOGICAL OPERATORS   ################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: MakeEdge
 * Description:  Make an Edge representation. This is a simple translation of MakeEdge 
 *               in Guibus and Stolfi (1985), p. 96.
 *--------------------------------------------------------------------------------------
 */
Edge *Triangulator::MakeEdge()
{
    QuadEdge *qe = new (m_dEdgePool->NewObject()) QuadEdge();
    
    qe->SetInUse();
    return qe->m_e;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: Splice
 * Description:  Splie two edges - as described in Guibus and Stolfi (1985), p. 98.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::Splice(Edge *a, Edge *b)
{
    Edge *ta, *tb;
    Edge *alpha = a->Onext()->Rot();
    Edge *beta  = b->Onext()->Rot();

    ta = a->Onext();
    tb = b->Onext();
    a->Onext_Set(tb);
    b->Onext_Set(ta);
    
    ta = alpha->Onext();
    tb = beta->Onext();
    alpha->Onext_Set(tb);
    beta->Onext_Set(ta);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: Connect
 * Description:  Connect two edges - as described in Guibus and Stolfi (1985), p. 103.
 *--------------------------------------------------------------------------------------
 */
Edge *Triangulator::Connect(Edge *a, Edge *b)
{
    Edge *e = MakeEdge();

    e->Org_Set(a->Dest());
    e->Dest_Set(b->Org());
    Splice(e, a->Lnext());
    Splice(e->Sym(), b);
    
    return e;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: Delete
 * Description:  Deletes an edge from the memory-pool.
 *--------------------------------------------------------------------------------------
 */
void Triangulator::Delete(Edge *e)
{
    Edge *f = e->Sym();
    if (e->Onext() != e) Splice(e, e->Oprev());
    if (f->Onext() != f) Splice(f, f->Oprev());
    
    e->Qedge()->SetFree();
    m_dEdgePool->DeleteObject(e->Qedge());
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: RightOf
 * Description:  Checks whether an Edge is on the right of another.
 *--------------------------------------------------------------------------------------
 */
int Triangulator::RightOf(Site *s, Edge *e)
{
    double result;

    Site::CCW(s, e->Dest(), e->Org(), &result);

    return result > 0.0;    
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Triangulator
 *      Method:  Triangulator :: LeftOf
 * Description:  Checks whether an Edge is on the left of another.
 *--------------------------------------------------------------------------------------
 */
int Triangulator::LeftOf(Site *s, Edge *e)
{
    double result;
    
    Site::CCW(s, e->Org(), e->Dest(), &result);
    
    return result > 0.0;    
}

}}

