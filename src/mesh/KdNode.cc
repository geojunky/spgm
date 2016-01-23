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
 *       Filename:  KdNode.cc
 *
 *    Description:  KdNode
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

#include <string.h>
#include <iostream>
#include <algorithm>

#include <KdNode.hh>
#include <KdItem.hh>
#include <KdTree.hh>

namespace src { namespace mesh {
using namespace std;

/*
 *--------------------------------------------------------------------------------------
 *       Class:  KdNode
 *      Method:  KdNode :: KdNode
 * Description:  Constructor through KdTree
 *--------------------------------------------------------------------------------------
 */
    KdNode::KdNode( KdTree *tree )
    {
        m_owner  = tree;

        m_upper = m_lower = NULL;
        m_left = m_right = NULL;

        m_currDim = 0;
        m_currSize = 0;
        m_slice = 0.;

        m_bucket = new vector< KdItem* >();
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: KdNode
     * Description:  Alternate constructor through parent
     *--------------------------------------------------------------------------------------
     */
    KdNode::KdNode( KdNode *node )
    {
        m_owner = node->m_owner;
        m_currDim = node->m_currDim + 1;
        m_currSize = 0;

        m_bucket = new vector< KdItem* >();

        if(( m_currDim+1 ) > 2)
        {
            m_currDim = 0;
        }

        m_upper = m_lower = NULL;
        m_left = m_right = NULL;
        m_slice = 0.;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Add
     * Description:  Adds a KdItem to the tree
     *--------------------------------------------------------------------------------------
     */
    void KdNode::Add( KdItem *m )
    {
        if( m_bucket == NULL )
        {
            if( m->m_coord[m_currDim] > m_slice )
            {
                m_right->Add( m );
            }
            else
            {
                m_left->Add( m );
            }
        }
        else
        {
            if(( m_currSize+1 ) > m_owner->m_bucketSize )
            {
                Split( m );
                return;
            }
            m_bucket->push_back( m );
            m_currSize++;
        }
        Expand( m->m_coord );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Split
     * Description:  Creates a spatial-cut in the tree
     *--------------------------------------------------------------------------------------
     */
    void KdNode::Split( KdItem *m )
    {
        m_slice = ( m_upper[m_currDim] + m_lower[m_currDim] ) / 2.;

        m_left = new KdNode( this );
        m_right = new KdNode( this );

        for( int i=0; i<m_currSize; i++ )
        {
            if( m_bucket->operator[]( i )->m_coord[m_currDim] > m_slice )
            {
                m_right->Add( m_bucket->operator[]( i ) );
            }
            else
            {
                m_left->Add( m_bucket->operator[]( i ) );
            }
        }

        m_currSize = 0;
        delete m_bucket;
        m_bucket = NULL;
        Add( m );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Expand
     * Description:  Expands the bounds of a node
     *--------------------------------------------------------------------------------------
     */
    void KdNode::Expand( double *newCoord )
    {
        if( m_upper == NULL )
        {
            m_upper = new double[2];
            memcpy( m_upper, newCoord, sizeof( double )*2 );

            m_lower = new double[2];
            memcpy( m_lower, newCoord, sizeof( double )*2 );

            return;
        }

        for( int i=0; i<2; i++ )
        {
            if( m_upper[i] < newCoord[i] )
            {
                m_upper[i] = newCoord[i];
            }

            if( m_lower[i] > newCoord[i] )
            {
                m_lower[i] = newCoord[i];
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Print
     * Description:  Prints nodes
     *--------------------------------------------------------------------------------------
     */
    void KdNode::Print()
    {
        if( m_left ) m_left->Print();
        if( m_right ) m_right->Print();

        if( m_bucket )
        {
            KdItem *it = NULL;

            std::cout << "ptr " << this << " num items " << m_currSize << std::endl;

            for( int i=0; i<m_currSize; i++ )
            {
                it = m_bucket->operator[]( i );

                it->Print();
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: DeleteHelper
     * Description:  Helper for deleting nodes
     *--------------------------------------------------------------------------------------
     */
    void KdNode::DeleteHelper()
    {
        if( m_left )
        {
            m_left->DeleteHelper();
        }

        if( m_right )
        {
            m_right->DeleteHelper();
        }

        if( m_bucket )
        {
            delete m_bucket;
            m_bucket = NULL;
        }

        if( m_upper )
        {
            delete [] m_upper;
            m_upper = NULL;
        }

        if( m_lower )
        {
            delete [] m_lower;
            m_lower = NULL;
        }

        if( m_left )
        {
            delete m_left;
            m_left = NULL;
        }

        if( m_right )
        {
            delete m_right;
            m_right = NULL;
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Intersects
     * Description:  Checks for intersection with input bounding-box
     *--------------------------------------------------------------------------------------
     */
    bool KdNode::Intersects( double *up0, double *low0, double *up1, double *low1 )
    {
        if( up0 && low0 && up1 && low1 )
        {
            for( int i=0; i<2; i++ )
            {
                if( up1[i] < low0[i] || low1[i] > up0[i] ) return false;
            }
            return true;
        }

        return false;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Range
     * Description:  Returns all the KdItems that fall within the given bounding-box
     *--------------------------------------------------------------------------------------
     */
    void KdNode::Range( double *upper, double *lower, vector<KdItem*> *result )
    {
        RangeHelper(upper, lower, result);

        sort(result->begin(), result->end(), m_itemSort);    
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: RangeHelper
     * Description:  Helper for Range-functon
     *--------------------------------------------------------------------------------------
     */
    void KdNode::RangeHelper( double *upper, double *lower, vector<KdItem*> *result )
    {
        if( m_bucket == NULL )
        {
            if( Intersects( upper, lower, m_left->m_upper, m_left->m_lower ) )
            {
                m_left->RangeHelper( upper, lower, result );
            }
            if( Intersects( upper, lower, m_right->m_upper, m_right->m_lower ) )
            {
                m_right->RangeHelper( upper, lower, result );
            }
            return;
        }
        for( int i = 0; i < m_currSize; i++ )
        {
            if( Contains( upper, lower, m_bucket->operator[]( i )->m_coord ) )
            {
                result->push_back( m_bucket->operator[]( i ) );
            }
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: Contains
     * Description:  Chekcs if a point resides within a given bounding-box
     *--------------------------------------------------------------------------------------
     */
    bool KdNode::Contains( double *upper, double *lower, double *pnt )
    {
        for( int i=0; i<2; i++ )
        {
            if( ( pnt[i] < upper[i] ) &&
                ( pnt[i] >= lower[i] ) )
            {
            }
            else
            {
                return false;
            }
        }

        return true;    
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdNode
     *      Method:  KdNode :: ~KdNode
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    KdNode::~KdNode()
    {
    }
}}
