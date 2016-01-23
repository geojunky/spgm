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
 *       Filename:  HillSlope.cc
 *
 *    Description:  Hillslope Processes
 *
 *        Version:  1.0
 *        Created:  18/05/14 14:41:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <SurfaceTopology.hh>
#include <SurfaceTopologyOutput.hh>
#include <HillSlope.hh>
#include <Model.hh>
#include <ScalarField.hh>
#include <Timer.hh>

namespace src{ namespace model {

    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  HillSlope
     *      Method:  HillSlope :: HillSlope
     * Description:  Constructor reads in parameters from config specific to this class.
     *--------------------------------------------------------------------------------------
     */
    HillSlope::HillSlope(const Model *m, Config *c)
    :Process(m, c)
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int len = st->GetNMeshPoints();
        /*-----------------------------------------------------------------------------
         * Read parameters
         *-----------------------------------------------------------------------------*/
        m_bedrockDiffusivity                = m_config->PDouble("bedrockDiffusivity");
        m_subaerialSedimentDiffusivity      = m_config->PDouble("subaerialSedimentDiffusivity");
        m_tolerance                         = m_config->PDouble("solverTolerance");
        m_maxIterations                     = m_config->PInt("maxIterations");

        /*-----------------------------------------------------------------------------
         * Instantiate diffusion solver
         *-----------------------------------------------------------------------------*/
        m_diffusion = new Diffusion(const_cast<SurfaceTopology*>(st), NULL, NULL, 
                                    m_model->GetNumTimeSteps(), m_model->GetDt(),
                                    m_tolerance, m_maxIterations);    

        /*-----------------------------------------------------------------------------
         * Register empty scalar-field for output
         *-----------------------------------------------------------------------------*/
        ScalarField<float> *diffusivity = new ScalarField<float>("diffusivity", len);
        SurfaceTopologyOutput *sto = m_model->GetSurfaceTopologyOutput();
        sto->RegisterScalarField(diffusivity);                
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  HillSlope
     *      Method:  HillSlope :: ~HillSlope
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    HillSlope::~HillSlope()
    {
        delete m_diffusion;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  HillSlope
     *      Method:  HillSlope :: Execute
     * Description:  Compute hill-slope erosion
     *--------------------------------------------------------------------------------------
     */
    void HillSlope::Execute()
    {
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int len = st->GetNMeshPoints();
        int nelem = st->GetNumTriangles();
        const unsigned int **triangles = st->GetTriangleIndices();
        
        if(m_model->GetTimeStep() % m_frequency) return;
        
        ScalarField<float> *sedimentHistory = static_cast< ScalarField<float>* > (m_model->GetField("sedimentHistory"));

        ScalarField<float> *diffusivity = new ScalarField<float>("diffusivity", len);
        /*-----------------------------------------------------------------------------
         * Set IC, Coefficients and Dirichlet values for diffusion-solver
         *-----------------------------------------------------------------------------*/
        vector<float> ic(len);
        vector<float> dirichlet(len);
        vector<float> coefficient(len);
        vector<float> solution(len);
        
        vector<float> elemCoefficient(nelem);

        for(int i=0; i<len; i++)
        {
            

            ic[i] = st->Z(i);
            if((int)(st->B(i)) == SurfaceTopology::DIRICHLET) 
            {
                dirichlet[i] = st->Z(i);
            }
            else
            {
                if(sedimentHistory)
                {
                    if((*sedimentHistory)(i)>0) 
                        coefficient[i] = m_subaerialSedimentDiffusivity;
                    else
                        coefficient[i] = m_bedrockDiffusivity;

                    (*diffusivity)(i) = coefficient[i];
                }
                else
                {
                    (*diffusivity)(i) = m_bedrockDiffusivity;
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Compute elemental coefficients 
         *-----------------------------------------------------------------------------*/
        for(int ie=0; ie<nelem; ie++)
        {
            const unsigned int *triIndices = triangles[ie];

            for(int i=0; i<3; i++)
            {
                elemCoefficient[ie] += coefficient[triIndices[i]];
            }
            elemCoefficient[ie] /= 3.0f;
        }

        m_diffusion->SetIC(&ic);
        m_diffusion->SetDirichlet(&dirichlet);
        m_diffusion->SetCoefficient(&elemCoefficient);

        m_diffusion->Step();
        m_diffusion->GetSolution(&solution);

        ScalarField<float> Z("z", len);
        
        for(int i=0; i<len; i++) Z(i) = solution[i] - st->Z(i);
        st->UpdateZ(&Z);

        /*-----------------------------------------------------------------------------
         * Register empty scalar-field for output
         *-----------------------------------------------------------------------------*/
        SurfaceTopologyOutput *sto = m_model->GetSurfaceTopologyOutput();
        sto->RegisterScalarField(diffusivity);        
    }
}}

