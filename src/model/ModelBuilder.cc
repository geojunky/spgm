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
 *       Filename:  ModelBuilder.cc
 *
 *    Description:  This class builds a Model based on the configuration file
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:36:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <ModelBuilder.hh>
#include <SurfaceTopology.hh>
#include <SurfaceTopologyOutput.hh>
#include <Precipitation.hh>
#include <FluvialErosion.hh>
#include <FluvialErosionDeposition.hh>
#include <Uplift.hh>
#include <HillSlope.hh>

namespace src { namespace model {
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    using namespace src::parser;
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  ModelBuilder
     *      Method:  ModelBuilder :: ModelBuilder
     * Description:  Constructor builds a Model object and instantiates all available 
     *               surface-processes.
     *--------------------------------------------------------------------------------------
     */
    ModelBuilder::ModelBuilder(Config *c)
    :m_config(c)
    {
        Config *meshConfig                  = m_config->Group("mesh");
        
        m_surfaceTopology                   = new SurfaceTopology(meshConfig);
        m_model                             = new Model(m_surfaceTopology, m_config);

        /* Instantiate writer for outputing mesh and drainage network */
        Config *oc = NULL;
        if((oc = c->Group("output")))
        {
            m_surfaceTopologyOutput = new SurfaceTopologyOutput(m_model, oc);
            m_model->RegisterSurfaceTopologyOutput(m_surfaceTopologyOutput);
        }
        
        Config *config                                   = NULL;

        if((config = m_config->Group("precipitation")))
        {
            Precipitation *p = new Precipitation(m_model, config);
            (void)p;
        }

        if((config = m_config->Group("fluvialErosion")))
        {
            FluvialErosion *dlft = new FluvialErosion(m_model, config);
            (void)dlft;
        }

        if((config = m_config->Group("fluvialErosionDeposition")))
        {
            FluvialErosionDeposition *tlft = new FluvialErosionDeposition(m_model, config);
            (void)tlft;
        }

        if((config = m_config->Group("uplift")))
        {
            Uplift *u = new Uplift(m_model, config);
            (void)u;
        }   

        if((config = m_config->Group("hillSlope")))
        {
            HillSlope *hs = new HillSlope(m_model, config);
            (void)hs;
        }   
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  ModelBuilder
     *      Method:  ModelBuilder :: ~ModelBuilder
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    ModelBuilder::~ModelBuilder()
    {
        delete m_surfaceTopology;
        delete m_model;
    }
}}

