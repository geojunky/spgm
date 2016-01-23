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
 *       Filename:  Uplift.cc
 *
 *    Description:  Uplift
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:35:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */

#include <vector>
#include <string>
#include <sstream>
#include <Uplift.hh>
#include <SurfaceTopology.hh>
#include <SurfaceTopologyOutput.hh>
#include <ScalarField.hh>
#include <Model.hh>
#include <Timer.hh>
#include <Log.hh>

namespace src { namespace model {
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Uplift
     *      Method:  Uplift :: Uplift
     * Description:  Constructor reads in parameters from config specific to this class.
     *--------------------------------------------------------------------------------------
     */
    Uplift::Uplift(const Model *m, Config *c)
    :Process(m, c)
    {
        /*-----------------------------------------------------------------------------
         * Instantiate time series for uplift-rate
         *-----------------------------------------------------------------------------*/
        m_upliftRate = new TimeSeries(m_model, m_config, "upliftRate");
        
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        int nn = st->GetNMeshPoints();
        ScalarField<float> *uplift = new ScalarField<float>("uplift", nn);

        m_model->AddField(uplift);
        m_cumulativeUplift.resize(nn);
        
        /*-----------------------------------------------------------------------------
         * Register empty scalar-field for output
         *-----------------------------------------------------------------------------*/
        SurfaceTopologyOutput *sto = m_model->GetSurfaceTopologyOutput();
        ScalarField<float> *u = new ScalarField<float> ("totalUplift", nn);
        sto->RegisterScalarField(u);        
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Uplift
     *      Method:  Uplift :: ~Uplift
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Uplift::~Uplift()
    {
        delete m_upliftRate;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Uplift
     *      Method:  Uplift :: Execute
     * Description:  All nodes except the boundary nodes are uplifted at a constant rate.
     *--------------------------------------------------------------------------------------
     */
    void Uplift::Execute()
    {
        if(m_model->GetTimeStep() % m_frequency) return;

        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        float dt = m_model->GetDt();
        ScalarField<float> *uplift = static_cast< ScalarField<float>* > (m_model->GetField("uplift"));
        int len = st->GetNMeshPoints();
        
        m_upliftRate->GetCurrentFieldValue(&m_upliftRateWorkArray);

        for(int i=0; i<len; i++) 
        {
            if(!st->B(i)) (*uplift)(i) = m_upliftRateWorkArray[i] * dt;
            else (*uplift)(i) = 0;
        }

        st->UpdateZ(uplift);
        for(int i=0; i<len; i++) m_cumulativeUplift[i] += (*uplift)(i);

        /*-----------------------------------------------------------------------------
         * Register scalar-field for output
         *-----------------------------------------------------------------------------*/
        SurfaceTopologyOutput *sto = m_model->GetSurfaceTopologyOutput();
        ScalarField<float> *u = new ScalarField<float> ("totalUplift", len);
           
        for(int i=0; i<len; i++) (*u)(i) = m_cumulativeUplift[i];
        sto->RegisterScalarField(u);              

#ifdef DEBUG
        cout << endl << "Nodal Uplift: " << endl;
        cout <<         "--------------------- " << endl;
        for(int i=0; i<len; i++)
        {
            cout << i << " " << (*uplift)( i ) << endl;
        }
#endif
    }
}}

