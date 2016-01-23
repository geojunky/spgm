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
 *       Filename:  SurfaceTopologyOutput.hh
 *
 *    Description:  This class implements output routines for SurfaceTopology
 *
 *        Version:  1.0
 *        Created:  24/02/14 13:57:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)                   
 *
 * =====================================================================================
 */
#ifndef SRC_MESH_SURFACE_TOPOLOGY_OUTPUT_HH
#define SRC_MESH_SURFACE_TOPOLOGY_OUTPUT_HH

#include <Model.hh>
#include <SurfaceTopology.hh>
#include <ScalarField.hh>
#include <Config.hh>

namespace src { namespace mesh {
    using namespace src::mem;
    using namespace src::mesh;
    using namespace src::model;
    using namespace src::parser;
    using namespace src::geometry;

    /*
     * =====================================================================================
     *        Class:  SurfaceTopologyOutput
     *  Description:  Implements output routines
     * =====================================================================================
     */
    class SurfaceTopologyOutput 
    {
        public:
        /*-----------------------------------------------------------------------------
         * Public interface 
         *-----------------------------------------------------------------------------*/
        friend class SurfaceTopology;

        typedef enum Attributes_t{
           SurfaceTopologyOutput_WriteMesh      = (1<<0),
           SurfaceTopologyOutput_WriteNetwork   = (1<<1),
        }Attributes;

        SurfaceTopologyOutput(const Model *m, Config *c);
        ~SurfaceTopologyOutput();

        void RegisterScalarField(ScalarField<float> *sf);

        void Write();
        
        /*-----------------------------------------------------------------------------
         * Private internals 
         *-----------------------------------------------------------------------------*/
        private:
        string m_prefix;
        string m_path;
        string m_outputFormat;
        int    m_frequency;
        int    m_timeStepOffset;
        bool   m_writeMesh;
        bool   m_writeDrainage;
        const Model *m_model;
        Config *m_config;
        const SurfaceTopology *m_surfaceTopology;
        void WriteVTK( float t, int ts);
        
        vector<ScalarField<float>*> m_registeredScalarFields;

        void WriteVTKMesh( float t, int ts);
        void WriteVTKDrainage( float t, int ts);
        template <class T>
        void WriteDataArray(ofstream &ofs, int nElem, T *data, int nComponents, string dataType, string name);
        
        void WriteTXT( float t, int ts);
    };
}}
#endif
