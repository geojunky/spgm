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
 *       Filename:  SurfaceTopology.hh
 *
 *    Description:  This class implements the mechanics described in sections 2 and 3
 *                  of Braun et al. (2013)
 *
 *        Version:  1.0
 *        Created:  24/02/14 13:21:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MESH_SURFACE_TOPOLOGY_HH
#define SRC_MESH_SURFACE_TOPOLOGY_HH

#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <math.h>

#include <MemoryPool.hh>
#include <Triangulator.hh>
#include <Config.hh>

#include <KdTree.hh>
#include <RegularMesh.hh>

namespace src { namespace util {
    template <class T> class ScalarField;
}}

namespace src { namespace mesh {
    using namespace src::mem;
    using namespace src::mesh;
    using namespace src::geometry;
    using namespace src::util;
    using namespace src::parser;
    using namespace std;

    /*
     * =====================================================================================
     *        Class:  SurfaceTopology
     *  Description:  Handles topological aspects of Badlands.
     * =====================================================================================
     */
    class SurfaceTopology
    {
        public:
        friend class Triangulator;
        friend class SurfaceTopologyOutput;
        
        static const int DIRICHLET;
        static const int NEUMANN;
        static const int CYCLIC;
        /*-----------------------------------------------------------------------------
         * Public interface 
         *-----------------------------------------------------------------------------*/
        SurfaceTopology(Config *c);
        ~SurfaceTopology();
        
        void InterpolateToRegularmesh(RegularMesh *rm, vector<float> &field) const;

        /*-----------------------------------------------------------------------------
         * Private internals 
         *-----------------------------------------------------------------------------*/
        private:
        static const int ORPHAN;
        static const int INVALID;

        Config *m_config;
        Triangulator *m_triangulator;
        string m_meshFileName;
        bool m_smoothing;
        float m_smoothingFactor;
        int   m_smoothingIterations;
        float **m_rawGeometry;
        float *m_z0; /* Initial height */
        float *m_zp; /* Height before last update */
        int m_nMeshPoints;
        float m_upper[2]; /* Upper bounding-box coords */
        float m_lower[2]; /* Lower bounding-box coords */
        vector<int> m_originalOrder;

        int *m_receivers;
        int *m_receiversSillCorrected;
        int *m_donorCounts;
        int **m_donors;
        int *m_donorsStorage;
        int *m_stack;
        int *m_catchmentIds;

        set<int> m_catchments; /* Set of catchments identified */
        float m_averageCellArea;

        float m_meshStartTime; /* Only applicable for meshes read from a .vtu file */

        void InitializeKdTree();
        void InitializeNetwork();
        void ValidateBoundaryConditions();
        void InitializeStack(int *index, int node, int catchmentId);
        void PropagateCatchmentTagUpstream(int node, int catchmentId);
        
        void ReadTextMesh(int *nMeshPoints, float ***points, float ***pointsSorted);
        void ReadVTUMesh(int *nMeshPoints, float ***points, float ***pointsSorted);
        float **ReadMeshGeometry(int *nMeshPoints);
        
        int CountOrphanNodes();
        void PrintMeshDetails();
        void PrintNetwork();

        /*-----------------------------------------------------------------------------
         * Public accessors
         *-----------------------------------------------------------------------------*/
        public:
        typedef set<int>::const_iterator CatchmentIterator;

        float GetAverageCellArea() const {return m_averageCellArea;}
        void GetBounds(vector<float> &upper, vector<float> &lower) const;
        void SavePreviousTimestep() const;
        inline const float *GetLowerBound(){ return m_lower; }
        inline unsigned int GetNMeshPoints() const {return m_nMeshPoints;}
        inline float X(int index) const {return m_rawGeometry[index][0];}   /* x-coordinate */
        inline float Y(int index) const {return m_rawGeometry[index][1];}   /* y-coordinate */
        inline float Z(int index) const {return m_rawGeometry[index][2];}   /* z-coordinate */
        inline float Z0(int index) const {return m_z0[index];}              /* z at time-step 0 */
        inline float Zp(int index) const {return m_zp[index];}              /* z at previous time-step */
        inline float B(int index) const {return m_rawGeometry[index][3];}   /* BC */
        
        inline int R(int index) const {return m_receivers[index];}          /* Receiver-id */
        inline int Dn(int index) const {return m_donorCounts[index];}       /* Donors-count */
        inline const int *D(int index) const {return m_donors[index];}      /* Donors-list */
        inline int C(int index) const {return m_catchmentIds[index];}       /* Catchment-Id */
        inline int S(int index) const {return m_stack[index];}              /* Stack-order */
        inline int O(int index) const {return m_originalOrder[index];}      /* original-order */
        /* Sill-corrected receivers */
        inline int SR(int index) const {return m_receiversSillCorrected[index];}

        /* Triangulation attributes */
        const unsigned int **GetTriangleIndices() const {return (const unsigned int **)m_triangulator->GetTriangleIndices();}
        const float **GetVoronoiSides() const {return (const float **) m_triangulator->GetVoronoiSides();}
        const float *GetVoronoiCellAreas() const {return (const float*) m_triangulator->GetVoronoiCellAreas();}
        const unsigned int *GetNumNeighbours() const {return (const unsigned int*) m_triangulator->GetNumNeighbours();}
        const unsigned int **GetNeighbours() const {return (const unsigned int**) m_triangulator->GetNeighbours();}
        const int *GetHull() const {return (const int*) m_triangulator->GetHull();}
        long int GetNumTriangles() const {return m_triangulator->GetNumTriangles();}
        long int GetNumFaces() const {return m_triangulator->GetNumFaces();}
        long int GetNumVoronoiVertices() const {return m_triangulator->GetNumVoronoiVertices();}
        const VSite *GetVoronoiVertices() const {return (const VSite*) m_triangulator->GetVoronoiVertices();}

        /* Update routines */
        void UpdateZ(ScalarField<float> *z) const;
        void UpdateNetwork();
        void PrintNode(int index) const;

        /* Spatial analytics */
        KdTree *m_kdTree;

        /* Catchment iterator */
        CatchmentIterator CatchmentsBegin() const { return m_catchments.begin(); };
        CatchmentIterator CatchmentsEnd()   const { return m_catchments.end(); };
    };
}}
#endif
