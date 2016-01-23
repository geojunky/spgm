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
 *       Filename:  FluvialErosion.hh
 *
 *    Description:  Implements a purely erosive stream-power law as described in 
 *                  Braun et al. (2013)
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:38:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_FLUVIAL_EROSION_HH
#define SRC_MODEL_FLUVIAL_EROSION_HH

#include <Process.hh>
#include <Config.hh>

namespace src{ namespace model {
    class Model;
}}

namespace src { namespace model {
    using namespace src::mesh;
    using namespace src::parser;
    using namespace std;
    
    /*
     * =====================================================================================
     *        Class:  FluvialErosion
     *  Description:  Implements purely erosive stream-power law
     * =====================================================================================
     */
    class FluvialErosion:public Process
    {
        public:
        FluvialErosion(const Model *m, Config *c);
        ~FluvialErosion();
        void Execute();
        
        private:
        void ComputeCatchmentArea();
        void SolveStreamPowerEquation();

        float m_Kf;
        float m_streamPowerM;
        float m_streamPowerN;
        float m_solverTolerance;
    };
}}

#endif
