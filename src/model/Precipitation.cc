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
 *       Filename:  Precipitation.cc
 *
 *    Description:  Uniform precipitation model
 *
 *        Version:  1.0
 *        Created:  24/02/14 14:14:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <Precipitation.hh>
#include <SurfaceTopology.hh>
#include <ScalarField.hh>
#include <Model.hh>
#include <Timer.hh>

namespace src { namespace model {
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Precipitation
     *      Method:  Precipitation :: Precipitation
     * Description:  Constructor reads in parameters from config specific to this class.
     *--------------------------------------------------------------------------------------
     */
    Precipitation::Precipitation(const Model *m, Config *c)
    :Process(m, c)
    {
        /*-----------------------------------------------------------------------------
         * Instantiate time series for precipitation-rate
         *-----------------------------------------------------------------------------*/
        m_precipitationRate = new TimeSeries(m_model, m_config, "precipitationRate");
        
        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        ScalarField<float> *precipitation = new ScalarField<float>("precipitation", st->GetNMeshPoints());

        m_model->AddField(precipitation);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Precipitation
     *      Method:  Precipitation :: ~Precipitation
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Precipitation::~Precipitation()
    {
        delete m_precipitationRate;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Precipitation
     *      Method:  Precipitation :: Execute
     * Description:  All nodes except the boundary nodes receive precipitation computed as 
     *               below.
     *--------------------------------------------------------------------------------------
     */
    void Precipitation::Execute()
    {
        if(m_model->GetTimeStep() % m_frequency) return;

        const SurfaceTopology *st = m_model->GetSurfaceTopology();
        const float *surfaceArea = st->GetVoronoiCellAreas();
        const int *hull = st->GetHull();
        float dt = m_model->GetDt();        
        ScalarField<float> *precipitation = static_cast< ScalarField<float>* > (m_model->GetField("precipitation"));
        int len = st->GetNMeshPoints();
        float averageCellArea = st->GetAverageCellArea();
        
        m_precipitationRate->GetCurrentFieldValue(&m_precipitationRateWorkArray);

        for(int i=0; i<len; i++)
            if(!hull[i]) (*precipitation)(i) = m_precipitationRateWorkArray[i] * surfaceArea[i] * dt;
            else (*precipitation)(i) = m_precipitationRateWorkArray[i] * averageCellArea * dt;

#ifdef DEBUG
        cout << endl << "Nodal Precipitation: " << endl;
        cout <<         "-------------------- " << endl;
        for(int i=0; i<len; i++)
        {
            cout << i << " " << (*precipitation)( i ) << endl;
        }
#endif
    }
}}

