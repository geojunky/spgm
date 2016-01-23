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
 *       Filename:  HillSlope.hh
 *
 *    Description:  Implements erosion/sedimentation by solving a nonlinear
 *                  diffusion equation, as described in Schlunegger et al. (2003)
 *
 *        Version:  1.0
 *        Created:  18/05/14 14:38:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_HILLSLOPE_HH
#define SRC_MODEL_HILLSLOPE_HH

#include <Process.hh>
#include <Config.hh>
#include <Diffusion.hh>

namespace src{ namespace model {
    class Model;
}}

namespace src { namespace model {
    using namespace src::mesh;
    using namespace src::parser;
    using namespace src::math;
    using namespace std;
    
    /*
     * =====================================================================================
     *        Class:  HillSlope
     *  Description:  Implements fluvial erosion/sedimentation
     * =====================================================================================
     */
    class HillSlope:public Process
    {
        public:
        HillSlope(const Model *m, Config *c);
        ~HillSlope();
        void Execute();
        
        static float DirichletFunction(int idx);
        static float CoefficientFunction(int idx);

        private:

        float m_bedrockDiffusivity;
        float m_subaerialSedimentDiffusivity;
        float m_submarineSedimentDiffusivity;
        double m_tolerance;
        int m_maxIterations;

        Diffusion *m_diffusion;
    };
}}

#endif
