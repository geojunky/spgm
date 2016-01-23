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
 *       Filename:  Timer.cc
 *
 *    Description:  Timer class
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:47:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <Timer.hh>
#include <omp.h>

namespace src { namespace util {
    using namespace std;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Timer
     *      Method:  Timer :: Timer
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    Timer::Timer()
    {
        #ifndef _OPENMP        
        m_time = clock();
        #else
        m_dtime = omp_get_wtime();
        #endif
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Timer
     *      Method:  Timer :: ~Timer
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Timer::~Timer(){}

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Timer
     *      Method:  Timer :: Elapsed
     * Description:  Compute the elapsed-time between two timer instances
     *--------------------------------------------------------------------------------------
     */
    double Timer::Elapsed(const Timer &begin, const Timer &end)
    {
        #ifndef _OPENMP        
        double diffticks = end.m_time - begin.m_time;
        
        double diffms    = (diffticks) / (CLOCKS_PER_SEC);
        return diffms;        
        
        #else
        return end.m_dtime - begin.m_dtime;
        #endif
    }
}}

