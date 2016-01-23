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
 *       Filename:  ScalarField.cc
 *
 *    Description:  Scalar field
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:40:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <ScalarField.hh>
#include <string.h>

using namespace std;
namespace src { namespace util {
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  ScalarField
     *      Method:  ScalarField :: ScalarField
     * Description:  Constrctor for a named scalar field.
     *--------------------------------------------------------------------------------------
     */
    template <class T>
    ScalarField<T>::ScalarField(string name, unsigned int length)
    :Field(name, length)
    {
        m_data = new T[m_length];
        memset(m_data, 0, sizeof(T)*m_length);
    };

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  ScalarField
     *      Method:  ScalarField :: ~ScalarField
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    template <class T>
    ScalarField<T>::~ScalarField()
    {
        delete [] m_data;
    }
    
    template class src::util::ScalarField<float>;
}
}
