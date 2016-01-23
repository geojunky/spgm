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
 *       Filename:  ModelBuilder.hh
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21/02/14 20:12:45
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)   
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_MODELBUILDER_HH
#define SRC_MODEL_MODELBUILDER_HH

#include <Config.hh>
#include <Log.hh>
#include <Model.hh>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

namespace src{
namespace mesh{
    class SurfaceTopologyOutput;
}}

using namespace std;
namespace src { namespace model {

using namespace src::parser;

/*
* =====================================================================================
*        Class:  ModelBuilder
*  Description:  This class builds a Model based on the configuration file.
* =====================================================================================
*/
class ModelBuilder
{
    public:
    ModelBuilder(Config *c);
    ~ModelBuilder();

    Model *GetModel()                       { return m_model; }
    SurfaceTopology *GetSurfaceTopology()   { return m_surfaceTopology; }
    SurfaceTopologyOutput *GetSurfaceTopologyOutput() { return m_surfaceTopologyOutput; }

    private:
    SurfaceTopology *m_surfaceTopology;
    SurfaceTopologyOutput *m_surfaceTopologyOutput;
    Model *m_model;
    Config *m_config;
};

}}

#endif
