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
 *       Filename:  Precipitation.hh
 *
 *    Description:  A simple uniform precipitation model
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:12:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_PRECIPITATION_HH
#define SRC_MODEL_PRECIPITATION_HH

#include <Process.hh>
#include <Model.hh>
#include <Config.hh>
#include <TimeSeries.hh>

namespace src { namespace model {
    using namespace src::mesh;
    using namespace src::parser;
    using namespace std;
    
    /*
     * =====================================================================================
     *        Class:  Precipitation
     *  Description:  Uniform precipitation model
     * =====================================================================================
     */
    class Precipitation:public Process
    {
        public:
        Precipitation(const Model *m, Config *c);
        ~Precipitation();
        void Execute();

        private:
        TimeSeries *m_precipitationRate;
        vector<double> m_precipitationRateWorkArray;
    };
}}

#endif
