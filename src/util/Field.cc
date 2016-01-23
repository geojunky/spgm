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
 *       Filename:  Field.cc
 *
 *    Description:  Base class
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:43:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <Field.hh>

using namespace std;
namespace src { namespace util {

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Field
     *      Method:  Field :: Field
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    Field::Field(string name, unsigned int length)
    :m_name(name),
    m_length(length)
    {
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Field
     *      Method:  Field :: ~Field
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Field::~Field(){}
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Field
     *      Method:  Field :: GetName
     * Description:  Returns the name of the field
     *--------------------------------------------------------------------------------------
     */
    string Field::GetName()
    {
        return m_name;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Field
     *      Method:  Field :: GetLength
     * Description:  Returns the length of the field
     *--------------------------------------------------------------------------------------
     */
    unsigned int Field::GetLength()
    {
        return m_length;
    }
}}

