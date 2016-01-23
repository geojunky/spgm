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
 *       Filename:  Model.hh
 *
 *    Description:  This class glues together all surface-processes.
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:05:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MODEL_PROCESSLIST_HH
#define SRC_MODEL_PROCESSLIST_HH

#include <MemoryPool.hh>
#include <string>
#include <map>
#include <vector>

namespace src{
namespace util{
    class Field;
}}

namespace src{
namespace mesh{
    class SurfaceTopology;
    class SurfaceTopologyOutput;
}}

namespace src{
namespace parser{
    class Config;
}}

namespace src { namespace model {
    using namespace std;
    using namespace src::model;
    using namespace src::mesh;
    using namespace src::util;
    using namespace src::parser;
    
    class Process;
    /*
     * =====================================================================================
     *        Class:  Model
     *  Description:  This class keeps track of all surface-processes and the 'fields' they
     *                create. 
     * =====================================================================================
     */
    class Model
    {
        public:
        Model(const SurfaceTopology *st, Config *c);
        ~Model();

        /*-----------------------------------------------------------------------------
         * Only dynamically allocated Fields should be passed to this function,
         * as it takes ownership of the fields and releases them at the end of
         * its life cycle.
         *-----------------------------------------------------------------------------*/
        void AddField(Field *f) const;
        /*-----------------------------------------------------------------------------
         * Only dynamically allocated Processes should be passed to this function,
         * as it takes ownership of the Processes and releases them at the end of
         * its life cycle.
         *-----------------------------------------------------------------------------*/
        void AddProcess(Process *p) const;
        
        Field* GetField(string name) const;
        Process* GetProcess(int index) const;
        int GetProcessCount() const;
        const SurfaceTopology *GetSurfaceTopology() const;
        SurfaceTopologyOutput *GetSurfaceTopologyOutput() const ;
        void RegisterSurfaceTopologyOutput(SurfaceTopologyOutput *sfo);
        float GetDt() const;
        float GetTime() const;
        int   GetTimeStep() const;
        int   GetNumTimeSteps(float t) const;
        int   GetNumTimeSteps() const;
        bool  NextTimeStep();
        int   GetNParallelCores() const;

        private:
        Config *m_config;
        const SurfaceTopology *m_surfaceTopology;
        mutable map<string, Field *> m_fields;
        mutable vector<Process *> m_processes;
        float m_dt;
        float m_t;
        int m_ts;
        int m_nts;
        float m_maxT;
        float m_beginT;

        int m_parallelCores;

        SurfaceTopologyOutput *m_surfaceTopologyOutput;
    };
}}

#endif
