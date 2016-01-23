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
 *       Filename:  Triangulator.hh
 *
 *    Description:  Triangulator interface.
 *
 *        Version:  1.0
 *        Created:  20/02/14 10:50:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_GEOMETRY_TRIANGULATOR_HH
#define SRC_GEOMETRY_TRIANGULATOR_HH

#include <Topology.hh>
#include <MemoryPool.hh>

namespace src { namespace geometry {
using namespace src::mem;
using namespace src::geometry;
/*
 * =====================================================================================
 *        Class:  Triangulator
 *  Description:  This class implements the divide and conquer algorithm described in 
 *                Guibas and Stolfi (1985), along with the dual Voronoi Diagram. Various
 *                attributes of a triangulation and its dual can be accessed, depending 
 *                on the initial object parameterization.
 * =====================================================================================
 */
class Triangulator
{
    public:
    /*-----------------------------------------------------------------------------
     * Constructor and Descructor 
     *-----------------------------------------------------------------------------*/
    Triangulator(int ns, float **s, unsigned int attr);
    ~Triangulator();

    /*-----------------------------------------------------------------------------
     * Public interface
     *-----------------------------------------------------------------------------*/
    unsigned int            **GetTriangleIndices();
    float                   **GetVoronoiSides();
    float                   *GetVoronoiCellAreas();
    unsigned int            *GetNumNeighbours();
    unsigned int            **GetNeighbours();
    int                     *GetHull();
    long int                GetNumTriangles();
    long int                GetNumFaces();
    long int                GetNumVoronoiVertices();
    VSite                   *GetVoronoiVertices();   
    friend std::ostream&    operator<<(std::ostream& os, const Triangulator& t);

    /*-----------------------------------------------------------------------------
     * Attributes used to initialize a Triangulator-object.
     * 1. Triangulator_SuperTriangle: Creates a super-triangle that contains all the
     *    input-points. This can be useful if the client needs all Voronoi-cell 
     *    areas to be bounded.
     * 2. Triangulator_TriangleIndices: Computes triangle-indices.
     * 3. Triangulator_TriangleNeighbours: Computes indices of neighbouring 
     *    triangles.
     * 4. Triangulator_VoronoiVertices: Computes the Voronoi-dual.
     * 5. Triangulator_VoronoiSides: Computes the lengths of the sides of Voronoi 
     *    cell.
     * 6. Triangulator_VoronoiCellAreas: Computes Voronoi-cell areas.
     * 7. Triangulator_NodeNeighbours: Computes the list of natural neighbours of 
     *    each node.
     *
     * Note: For the computation of voronoi-attributes, naturally, the 
     * Triangulator_VoronoiVertices flag must be enabled.
     *-----------------------------------------------------------------------------*/
    typedef enum Attributes_t
    {
        Triangulator_SuperTriangle          = (1<<0),
        Triangulator_TriangleIndices        = (1<<1),
        Triangulator_TriangleNeighbours     = (1<<2),
        Triangulator_VoronoiVertices        = (1<<3),
        Triangulator_VoronoiSides           = (1<<4),
        Triangulator_VoronoiCellAreas        = (1<<5),
        Triangulator_NodeNeighbours         = (1<<6)
    }Attributes;
    
    void ComputeBound( float *minx, float *miny, float *maxx, float *maxy );
    
    private:
    /*-----------------------------------------------------------------------------
     * Private internals
     *-----------------------------------------------------------------------------*/
    static double const PI;
    /*-----------------------------------------------------------------------------
     * Member variables 
     *-----------------------------------------------------------------------------*/
    unsigned int                m_attributes;
    int                         m_nSites;
    int                         m_nInputSites;
    Site                        *m_sites;
    float                       **m_superTriangle;
    Edge                        *m_le;
    Edge                        *m_re;
    MemoryPool                  *m_dEdgePool;
    MemoryPool                  *m_vEdgePool;

    /* Edges and faces */
    int                         m_nEdges;
    int                         m_nVoronoiSites;
    int                         m_nTriangles;
    int                         m_nFaces;
    unsigned int                **m_tIndices;
    unsigned int                **m_tNeighbours;
    int                         m_nVoronoiVertices;
    unsigned int                *m_nNeighbours;
    unsigned int                **m_neighbours;
    float                       **m_voronoiSides;
    float                       *m_voronoiArea;
    int                         *m_hull;
    int                         *m_outputHull;

    /*-----------------------------------------------------------------------------
     * Member functions
     *-----------------------------------------------------------------------------*/
    void AllocateStorage();
    void SortSites();
    void InitSuperTriangle();
    void Delaunay( int sl, int sh, Edge **le, Edge **re );   

    private:
    /*-----------------------------------------------------------------------------
     * Internal functions
     *-----------------------------------------------------------------------------*/
    void MarkHull();
    void GenerateTriangleIndices();
    void GenerateVoronoiVertices();
    void GenerateNodeNeighbours();

    /*-----------------------------------------------------------------------------
     * Topological operators
     *-----------------------------------------------------------------------------*/
    Edge *MakeEdge();
    void Splice(Edge *a, Edge *b);
    void Delete(Edge *e);
    Edge *Connect(Edge *a, Edge *b);
    
    int RightOf(Site *s, Edge *e);
    int LeftOf(Site *s, Edge *e);
};

}}
#endif
