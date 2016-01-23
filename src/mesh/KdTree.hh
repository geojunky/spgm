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
 *       Filename:  KdTree.hh
 *
 *    Description:  KdTree implementation
 *
 *        Version:  1.0
 *        Created:  11/06/14 14:02:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)         
 *
 * =====================================================================================
 */
#ifndef SRC_MESH_KDTREE_HH
#define SRC_MESH_KDTREE_HH

#include <Timer.hh>
#include <KdItem.hh>
#include <KdNode.hh>
#include <MemoryPool.hh>

namespace src { namespace mesh {
    using namespace std;
    using namespace src::mem;

    /*
     * =====================================================================================
     *        Class:  KdTree
     *  Description:  kd-tree definition for spatial analysis
     * =====================================================================================
     */
    class KdTree
    {
        public:

        friend class src::mesh::KdNode;

        /*-----------------------------------------------------------------------------
         * Public interface
         *-----------------------------------------------------------------------------*/
        KdTree( int maxElems );

        void Add( float *c, int id );

        void Print();

        void DeleteAll();

        int Size();

        void QueryBallPoint(float *pos, float r, vector<float> *distance, vector<int> *id);

        ~KdTree();

        private:
        
        /*-----------------------------------------------------------------------------
         * Private internals
         *-----------------------------------------------------------------------------*/
        void Range( double *upper, double *lower, vector<KdItem*> *result );

        int m_bucketSize;
        int m_itemCount;
        int m_maxElems;

        KdNode *m_root;

        /* pool */
        MemoryPool *m_pool;
    };

}}
#endif

