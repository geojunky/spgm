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
 *       Filename:  Uplift.hh
 *
 *    Description:  Uplift
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:34:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_UPLIFT_HH
#define SRC_MODEL_UPLIFT_HH

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
     *        Class:  Uplift
     *  Description:  Uniform uplift
     * =====================================================================================
     */
    class Uplift:public Process
    {
        public:
        Uplift(const Model *m, Config *c);
        ~Uplift();
        void Execute();

        private:
        TimeSeries *m_upliftRate;
        vector<double> m_upliftRateWorkArray;
        vector<float> m_cumulativeUplift;
    };
}}

#endif
