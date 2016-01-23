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
 *       Filename:  Topology.hh
 *
 *    Description:  Some of the basic building blocks of a Delaunay Triangulation and 
 *                  its dual, the Voronoi Diagram, are defined and implemented in this
 *                  file.
 *
 *        Version:  1.0
 *        Created:  20/02/14 09:45:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_GEOMETRY_TOPOLOGY_HH
#define SRC_GEOMETRY_TOPOLOGY_HH

#include <iostream>
#include <stdlib.h>
#include <string.h>

namespace src { namespace geometry {
using namespace std;

class Triangulator;
class QuadEdge;
class Site;

/*-----------------------------------------------------------------------------
 * Simple utility functions 
 *-----------------------------------------------------------------------------*/
inline float FABS(float a) {return ((a) >= 0.0 ? (a) : -(a));}

/*
 * =====================================================================================
 *        Class:  VSite
 *  Description:  This class represents a spatial location of a voronoi node.
 * =====================================================================================
 */
class VSite
{
    public:
    float m_coord[2];
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  operator<<
     *  Description:  Prints the coordinates of a VSite.
     * =====================================================================================
     */
    friend std::ostream& operator<<(std::ostream& os, const VSite& mp) 
    {
        os << "VSite: (" << (mp.m_coord)[0] << 
              ", " << (mp.m_coord)[1] << ")" << endl;
        return os;
    }    
};

/*
 * =====================================================================================
 *        Class:  Site
 *  Description:  This class represents the spatial location of a node in the Delaunay
 *                triangulation. Note, the spatial coordinates of each node are not
 *                explicitly stored, only pointers to them are stored.
 * =====================================================================================
 */
class Site
{
    public:
    float **m_coord;
    unsigned int m_id;
    
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  InCircle
     *  Description:  Test whether the point 'd' lies within the circumcirle formed by 
     *                points 'a', 'b' and 'c'
     * =====================================================================================
     */
    inline static int InCircle(Site *_a, Site *_b, Site *_c, Site *_d)
    {
        float *a = *(_a->m_coord);
        float *b = *(_b->m_coord);
        float *c = *(_c->m_coord);
        float *d = *(_d->m_coord);
        
        double x1 = a[0], y1 = a[1];
        double x2 = b[0], y2 = b[1];
        double x3 = c[0], y3 = c[1];
        double x4 = d[0], y4 = d[1];

        return ((y4-y1)*(x2-x3)+(x4-x1)*(y2-y3))*((x4-x3)*(x2-x1)-(y4-y3)*(y2-y1)) >
        	   ((y4-y3)*(x2-x1)+(x4-x3)*(y2-y1))*((x4-x1)*(x2-x3)-(y4-y1)*(y2-y3));
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  CCW
     *  Description:  Returns twice the area of an oriented triangle formed by points 'a', 
     *                'b' and 'c'. The area is positive if the triangle is oriented 
     *                anticlockwise.
     * =====================================================================================
     */
    inline static void CCW(Site *_a, Site *_b, Site *_c, double *result)
    {
        float *a = *(_a->m_coord);
        float *b = *(_b->m_coord);
        float *c = *(_c->m_coord);
        
        double ax = a[0];  double ay = a[1];
		double  bx = b[0]; double by = b[1];
		double  cx = c[0]; double cy = c[1];
		*(result) = ((bx*cy-by*cx) - (ax*cy-ay*cx) + (ax*by-ay*bx));
    }
    
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  CCW
     *  Description:  The same as the above function, except that points 'a' and 'b'
     *                represent Voronoi nodes.
     * =====================================================================================
     */
    inline static void CCW(Site *_a, VSite *_b, VSite *_c, double *result)
    {
        float *a = *(_a->m_coord);
        float *b = _b->m_coord;
        float *c = _c->m_coord;
        
        double ax = a[0];  double ay = a[1];
		double  bx = b[0]; double by = b[1];
		double  cx = c[0]; double cy = c[1];
		*(result) = ((bx*cy-by*cx) - (ax*cy-ay*cx) + (ax*by-ay*bx));
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  Circumcenter
     *  Description:  Computes the circumcenter of a triangle formed by points 'a', 'b'
     *                and 'c'.
     * =====================================================================================
     */
    inline static bool Circumcenter( const Site* s1, const Site* s2,const Site* s3, VSite *vs)
    {
        float *n1 = *(s1->m_coord);
        float *n2 = *(s2->m_coord);
        float *n3 = *(s3->m_coord);

        double x1 = n1[0];
        double y1 = n1[1];

        double x2 = n2[0];
        double y2 = n2[1];

        double x3 = n3[0];
        double y3 = n3[1];

        // Distances relative to point n1 of the triangle
        double x21 = x2 - x1;
        double y21 = y2 - y1;
        double x31 = x3 - x1;
        double y31 = y3 - y1;

        double denominator = 0.5 / (x21 * y31 - y21 * x31);

        if (denominator == 0) 
        {
            // Degenerate triangle
            cerr << "Error: Degenerate triangle encountered.." << endl;
            return false;
        }

        // Squares of the lengths of the edges incident to n1
        double length21 = x21 * x21 + y21 * y21;
        double length31 = x31 * x31 + y31 * y31;

        // Calculate offset from n1 of circumcenter
        double x = (y31 * length21 - y21 * length31) * denominator;  
        double y = (x21 * length31 - x31 * length21) * denominator;

        // Create a node at the circumcenter
        vs->m_coord[0] = x + x1;
        vs->m_coord[1] = y + y1;

        // Circumcenter calculated
        return true;
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  operator<<
     *  Description:  Prints the coordinated of a Site.
     * =====================================================================================
     */
    friend std::ostream& operator<<(std::ostream& os, const Site& mp) 
    {
        os << "Site: id(" << mp.m_id << "): " << "(" << 
              (*mp.m_coord)[0] << ", " << (*mp.m_coord)[1] << ")" << endl;
        return os;
    }    
};

/*
 * =====================================================================================
 *        Class:  SiteComparator
 *  Description:  A functor for sorting spatial coordinates in ascending x-order, 
 *                taking y into account only when the x-coordinates are equal.
 * =====================================================================================
 */
class SiteComparator
{
    public:
    bool operator()(const Site &a, const Site &b)
    {
        float *coordA = (*(a.m_coord));
        float *coordB = (*(b.m_coord));

        if(coordA[0] != coordB[0])
            return coordA[0] < coordB[0];
        else
            return coordA[1] < coordB[1];
    }
};

/*
 * =====================================================================================
 *        Class:  QuadEdge
 *  Description:  The QuadEdge data-structure was adapted from Dani Lischinski's initial
 *                implementation as outlined in p. 48 of Graphics Gems IV, by Paul 
 *                Heckbert (1994). This class implements the basic topological operators
 *                outlined in Guibas and Stolfi (1985).
 * =====================================================================================
 */
class QuadEdge;
class Edge 
{
    friend class QuadEdge;
    
    private:
    int m_num;
    Edge *m_next;
    void *m_data[2];
    public:
    Edge() { m_data[0] = m_data[1] = 0; }
    
    /*-----------------------------------------------------------------------------
     * The basic Edge-algebra below are described in great detail in 
     * Guibas and Stolfi (1985).
     *-----------------------------------------------------------------------------*/
    inline Edge* Rot() { return (m_num < 3) ? this + 1 : this - 3; }

    inline Edge* Tor() { return (m_num > 0) ? this - 1 : this + 3; }

    inline Edge* Sym() { return (m_num < 2) ? this + 2 : this - 2; }

    inline Edge* Onext() { return m_next; }

    inline Edge* Oprev() { return Rot()->Onext()->Rot(); }

    inline Edge* Dnext() { return Sym()->Onext()->Sym(); }

    inline Edge* Dprev() { return Tor()->Onext()->Tor(); }

    inline Edge* Lnext() { return Tor()->Onext()->Rot(); }

    inline Edge* Lprev(){ return Onext()->Sym(); }

    inline Edge* Rnext() { return Rot()->Onext()->Tor(); }

    inline Edge* Rprev() { return Sym()->Onext(); }

    inline Site* Org() { return (Site*)m_data[0]; }

    inline Site* Dest() { return Sym()->Org(); }

    inline VSite* VOrg() { return (VSite*)m_data[1]; }

    inline VSite* VDest() { return Sym()->VOrg(); }
    
    QuadEdge* Qedge() { return (QuadEdge *)(this - m_num); }

    /*-----------------------------------------------------------------------------
     * Setters 
     *-----------------------------------------------------------------------------*/
    inline void Onext_Set(Edge *e) { m_next = e; }
    inline void Org_Set(Site *s) { m_data[0] = s; }
    inline void Dest_Set(Site *s) { Sym()->Org_Set(s); }
    inline void VOrg_Set(VSite *vs) { m_data[1] = vs; }
    inline void VDest_Set(VSite *vs) { Sym()->VOrg_Set(vs); }
};


/*
 * =====================================================================================
 *        Class:  QuadEdge
 *  Description:  Representation of an Edge in a Delaunay Triangulation. Each edge is
 *                represented by a group of 4 edges, the canonical representative of 
 *                each group is m_e[0]. See Guibas and Stolfi (1985) p. 92 for more 
 *                details.
 * =====================================================================================
 */
class QuadEdge 
{
    friend class Triangulator;
    private:
    Edge m_e[4];
    int m_attributes;
    public:
    
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  QuadEdge
     *  Description:  Constructor initializes edge-orientations are outlined in function
     *                MakeEdge in Stolfi (1985) p. 96.
     * =====================================================================================
     */
    inline QuadEdge()
    {
        m_e[0].m_num = 0, m_e[1].m_num = 1, m_e[2].m_num = 2, m_e[3].m_num = 3;
        m_e[0].m_next = &(m_e[0]); m_e[1].m_next = &(m_e[3]);
        m_e[2].m_next = &(m_e[2]); m_e[3].m_next = &(m_e[1]);

        m_attributes = 0;
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  Visited
     *  Description:  The attribute m_attributes encodes two bits of information:
     *                1. The number of times a node has been visited, which is useful for 
     *                   traversing edges to compute various things e.g. the 
     *                   triangle-indices - marking off edges that have already been 
     *                   visited.
     *                2. Whether an Edge object is currently in use, since Edges are 
     *                   bulk-allocated in a pool of memory. A simple way to traverse all 
     *                   edges after the triangulation has been computed is to go 
     *                   sequentially through all the edges in the memory-pool, skipping the
     *                   ones that are marked as 'free'.
     *
     *                This function returns how many times an Edge has been visited by 
     *                masking out the lower 16 bits.
     * =====================================================================================
     */
    inline int Visited()
    {
        return (m_attributes & 0xffff0000) >> 16;
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  ResetVisited
     *  Description:  Resets the number of times an Edge has been visited.
     * =====================================================================================
     */
    inline void ResetVisited()
    {
        m_attributes &= ~0xffff0000;
    }

    
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  IncrementVisited
     *  Description:  Increments the visit-count.
     * =====================================================================================
     */
    inline void IncrementVisited()
    {
        int val = (m_attributes & 0xffff0000) >> 16;
        m_attributes &= ~0xffff0000;
        val++;
        m_attributes |= (val<<16);
    }
    
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  DecrementVisited
     *  Description:  Decrements the visit-count.
     * =====================================================================================
     */
    inline void DecrementVisited()
    {
        int val = (m_attributes & 0xffff0000) >> 16;
        m_attributes &= ~0xffff0000;
        val--;
        m_attributes |= (val<<16);
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  IsFree
     *  Description:  Returns if an Edge is set to 'free'.
     * =====================================================================================
     */
    inline bool IsFree()
    {
        return (m_attributes & 0x0000ffff)==0;
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  SetFree
     *  Description:  Sets an Edge to 'free'.
     * =====================================================================================
     */
    inline void SetFree()
    {
        m_attributes &= ~0x0000ffff;
    }

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  SetInUse
     *  Description:  Sets an Edge to 'not-free'
     * =====================================================================================
     */
    inline void SetInUse()
    {
        m_attributes |= 1;
    }
};

}}
#endif
