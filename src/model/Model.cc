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
 *       Filename:  Model.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:10:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */

#include <iostream>
#include <unistd.h>

#include <Model.hh>
#include <SurfaceTopology.hh>
#include <Timer.hh>

#include <Field.hh>
#include <Process.hh>
#include <Config.hh>
#include <Log.hh>

namespace src { namespace model {
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    using namespace src::parser;
    

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: Model
     * Description:  Constructor reads in parameters from the root config.
     *--------------------------------------------------------------------------------------
     */
    Model::Model(const SurfaceTopology *st, Config *c)
    :m_config(c),
    m_surfaceTopology(st)
    {
        /*-----------------------------------------------------------------------------
         * Read parameters
         *-----------------------------------------------------------------------------*/
        m_dt        = m_config->PDouble("dt");
        m_maxT      = m_config->PDouble("maxTime");
        m_beginT    = m_config->PDouble("beginTime");
        m_parallelCores  = m_config->PInt("parallelCores");

        m_t = m_beginT;
        m_ts = GetNumTimeSteps(m_t);
        m_nts = GetNumTimeSteps(m_maxT);

        /*-----------------------------------------------------------------------------
         * If m_maxProcs==-1, retrieve number of cores on the machine
         *-----------------------------------------------------------------------------*/
        int coresAvailable = (int)sysconf(_SC_NPROCESSORS_ONLN);
        
        if(m_parallelCores == -1)
        {
            m_parallelCores = coresAvailable;
        }
        else if((m_parallelCores<=0) || (m_parallelCores>coresAvailable))
        {
            LogError(cout << "Error: Number of parallel cores must be between 1 and " << coresAvailable << endl);
            exit(EXIT_FAILURE);
        }
        
        omp_set_num_threads(m_parallelCores);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: AddField
     * Description:  Surface-processes register the fields they create via this function so
     *               they can be accessed by other surface-processes that might need them.
     *--------------------------------------------------------------------------------------
     */
    void Model::AddField(Field *f) const
    {
        map<string, Field*>::iterator result = m_fields.find(f->GetName());

        if(result == m_fields.end()) /* Field does not exist */
        {
            m_fields.insert(make_pair<string, Field*>(f->GetName(), f));
        }
        else
        {
#ifdef DEBUG
            cout << "Error in src:Model.cc line " << __LINE__ << ". Field already exists.";
#endif
        }
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: AddProcess
     * Description:  Processes register themselves to Model via this function.
     *--------------------------------------------------------------------------------------
     */
    void Model::AddProcess(Process *p) const
    {
        m_processes.push_back(p);
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetField
     * Description:  Returns a named field.
     *--------------------------------------------------------------------------------------
     */
    Field* Model::GetField(string name) const
    {
        map<string, Field*>::iterator result = m_fields.find(name);

        if(result == m_fields.end()) return NULL;

        return result->second;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetProcessCount
     * Description:  Returns the number of surface-processes.
     *--------------------------------------------------------------------------------------
     */
    int Model::GetProcessCount() const
    {
        return m_processes.size();
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetProcess
     * Description:  Returns a process.
     *--------------------------------------------------------------------------------------
     */
    Process* Model::GetProcess(int index) const
    {
        if(index < (int)m_processes.size()) return m_processes[index];

        return NULL;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetSurfaceTopology
     * Description:  Returns the SurfaceTopology
     *--------------------------------------------------------------------------------------
     */
    const SurfaceTopology* Model::GetSurfaceTopology() const
    {
        return m_surfaceTopology;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: RegisterSurfaceTopologyOutput
     * Description:  Registers SurfaceTopologyOutput
     *--------------------------------------------------------------------------------------
     */
    void Model::RegisterSurfaceTopologyOutput(SurfaceTopologyOutput *sfo)
    {
        m_surfaceTopologyOutput = sfo;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetSurfaceTopologyOutput
     * Description:  Returns the SurfaceTopologyOutput
     *--------------------------------------------------------------------------------------
     */
    SurfaceTopologyOutput * Model::GetSurfaceTopologyOutput() const
    {
        return m_surfaceTopologyOutput;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetDt
     * Description:  Returns dt
     *--------------------------------------------------------------------------------------
     */
    float Model::GetDt() const
    {
        return m_dt;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetTime
     * Description:  Returns the current model-time
     *--------------------------------------------------------------------------------------
     */
    float Model::GetTime() const
    {
        return m_t;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetTimeStep
     * Description:  Returns the current time-step
     *--------------------------------------------------------------------------------------
     */
    int Model::GetTimeStep() const
    {
        return m_ts;
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetNumTimeSteps
     * Description:  Returns number of time-steps
     *--------------------------------------------------------------------------------------
     */
    int Model::GetNumTimeSteps(float t) const
    {
        return (int)(floor(t/m_dt));
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetNumTimeSteps
     * Description:  Returns the total number of time-steps
     *--------------------------------------------------------------------------------------
     */
    int Model::GetNumTimeSteps() const
    {
        return (int)(floor(m_maxT/m_dt));
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: GetNParallelCores
     * Description:  Returns the total number of processors
     *--------------------------------------------------------------------------------------
     */
    int Model::GetNParallelCores() const
    {
        return m_parallelCores;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: NextTimeStep
     * Description:  Returns true if current model-time is <= max model-time.
     *--------------------------------------------------------------------------------------
     */
    bool Model::NextTimeStep()
    {
        m_t += m_dt;
        m_ts++;
        
        m_surfaceTopology->SavePreviousTimestep();
        
        if(m_ts <= m_nts) return true;

        return false;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Model
     *      Method:  Model :: ~Model
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    Model::~Model()
    {
        for(map<string, Field*>::iterator it = m_fields.begin();
            it != m_fields.end(); it++)
        {
            delete it->second;
        }

        for(vector<Process *>::iterator it = m_processes.begin();
            it != m_processes.end(); it++)
        {
            delete *it;
        }
    }
}}

