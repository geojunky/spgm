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
 *       Filename:  ScalarField.hh
 *
 *    Description:  Scalar field
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:39:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_UTIL_SCALARFIELD_HH
#define SRC_UTIL_SCALARFIELD_HH

#include <Field.hh>

using namespace std;

namespace src { namespace util {

    /*
     * =====================================================================================
     *        Class:  ScalarField
     *  Description:  A simple templated scalar-field.
     * =====================================================================================
     */
    template <class T>
    class ScalarField : public Field
    {
        public:
        ScalarField(string name, unsigned int length);
        ~ScalarField();
        
        inline T& operator()(unsigned int index)
        {
            return m_data[index];
        }
            
        private:
        T *m_data;
    };
}}

#endif
