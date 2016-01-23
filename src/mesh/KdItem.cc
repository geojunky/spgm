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
 *       Filename:  KdItem.cc
 *
 *    Description:  KdTree Items
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

#include <iostream>
#include <KdItem.hh>

namespace src { namespace mesh {
using namespace std;

/*
 *--------------------------------------------------------------------------------------
 *       Class:  KdItem
 *      Method:  KdItem :: KdItem
 * Description:  Constructor
 *--------------------------------------------------------------------------------------
 */
    KdItem::KdItem()
    {
        m_coord[0] = 0;
        m_coord[1] = 0;
        m_id = -1;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdItem
     *      Method:  KdItem :: Print
     * Description:  Prints KdItem
     *--------------------------------------------------------------------------------------
     */
    void KdItem::Print()
    {
        std::cout << "[";
        for( int i=0; i<2; i++ )
        {
            std::cout << m_coord[i];

            if((( i+1 ) != 2 ) )
                std::cout << ", ";
        }
        std::cout << "]";

        std::cout << " id - " << m_id << std::endl;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  KdItem
     *      Method:  KdItem :: ~KdItem
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    KdItem::~KdItem()
    {
        delete [] m_coord;
    }
}}
