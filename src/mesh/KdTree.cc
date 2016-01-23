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
 *       Filename:  KdTree.cc
 *
 *    Description:  D-dimensional tree implementation
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

#include <KdTree.hh>
#include <math.h>
namespace src { namespace mesh {
using namespace std;
    
    const int DEFAULT_CHUNK_SIZE = 2048;

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: KdTree
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    KdTree::KdTree( int maxElems=0 )
    {
        m_bucketSize = 16;
        m_itemCount = 0;
        m_maxElems = maxElems;

        m_root = new KdNode( this );

        /* Initialize pool */
        if(m_maxElems==0)
        {
            m_pool = new MemoryPool(sizeof(KdItem), DEFAULT_CHUNK_SIZE);
        }
        else
        {
            m_pool = new MemoryPool(sizeof(KdItem), m_maxElems, MemoryPool::Fixed);
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: Add
     * Description:  Adds points to the kd-tree
     *--------------------------------------------------------------------------------------
     */
    void KdTree::Add( float *c, int id )
    {
        KdItem *ki = new (m_pool->NewObject()) KdItem();

        ki->m_coord[0] = c[0];
        ki->m_coord[1] = c[1];
        ki->m_id = id;

        if( m_root == NULL )
        {
            m_root = new KdNode( this );
        }

        m_root->Add( ki );

        m_itemCount++;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: Print
     * Description:  Prints kd-tree nodes
     *--------------------------------------------------------------------------------------
     */
    void KdTree::Print()
    {
        if( m_root )
        {
            m_root->Print();
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: Range
     * Description:  Returns all the KdItems that are within [lower[..], upper[..]]
     *--------------------------------------------------------------------------------------
     */
    void KdTree::Range( double *upper, double *lower, vector<KdItem*> *result )
    {
        if( m_root )
        {
            m_root->Range( upper, lower, result );
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: DeleteAll
     * Description:  Deletes all entries
     *--------------------------------------------------------------------------------------
     */
    void KdTree::DeleteAll()
    {
        if( m_root )
        {
            m_root->DeleteHelper();
            delete m_root;

            m_root = NULL;
        }

        m_itemCount = 0;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: Size
     * Description:  Returns the size of the kd-tree
     *--------------------------------------------------------------------------------------
     */
    int KdTree::Size()
    {
        return m_itemCount;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: QueryBallPoint
     * Description:  Returns the ids that fall within the r-sphere, along with corresponding
     *               distances
     *--------------------------------------------------------------------------------------
     */
    void KdTree::QueryBallPoint(float *pos, float r, vector<float> *distance, vector<int> *id)
    {
        vector<KdItem*> initialResults;
        double upper[2];
        double lower[2];

        for(int i=0; i<2; i++) 
        {
            lower[i] = pos[i] - r;
            upper[i] = pos[i] + r;
        }

        Range(upper, lower, &initialResults);

        /*-----------------------------------------------------------------------------
         * Filter out points that lie outside the r-sphere
         *-----------------------------------------------------------------------------*/
        distance->clear();
        id->clear();
        for(vector<KdItem*>::iterator rit = initialResults.begin();
            rit != initialResults.end(); rit++)
        {
            double dist = 0.;

            for(int i=0; i<2; i++)
            {
                double diff = (*rit)->m_coord[i] - pos[i];
                dist += diff*diff;
            }

            dist = sqrt(dist);

            if(dist < r)
            {
                distance->push_back(dist);
                id->push_back((*rit)->m_id);
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdTree
     *      Method:  KdTree :: ~KdTree
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    KdTree::~KdTree()
    {
        DeleteAll();
        delete m_pool;
    }
}}
