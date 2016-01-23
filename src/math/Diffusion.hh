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
 *       Filename:  Diffusion.hh
 *
 *    Description:  Implementation of 2D Diffusion using the FEM
 *
 *        Version:  1.0
 *        Created:  13/05/14 16:01:19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)
 *        Company:  
 *
 * =====================================================================================
 */
#include <vector>
#include <SurfaceTopology.hh>
#include <ScalarField.hh>
#include <Timer.hh>

#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace src{ namespace math {
    using namespace Eigen;
    using namespace std;
    using src::mesh::SurfaceTopology;
    /*
     * =====================================================================================
     *        Class:  Diffusion
     *  Description:  2D Diffusion on a Triangular Mesh. The nonlinear diffusion equation
     *                is cast in FE form using P1 triangular elements and the resulting 
     *                algebraic equations are solved using a Conjugate Gradient solver in 
     *                the Eigen library.
     * =====================================================================================
     */
    class Diffusion
    {
        public:
        
        typedef struct Coord_t{ float x; float y;
                                Coord_t(float _x, float _y):x(_x),y(_y){} }Coord;
        
        typedef float (*ForcingFunc)     (float, float, float);
        typedef float (*NeumannFunc)     (float, float, float);

        Diffusion( SurfaceTopology *st, ForcingFunc f, NeumannFunc n, int nt, float dt, 
                   double tolerance, int maxIterations );
        ~Diffusion();
        
        void SetIC(vector<float> *vals);
        void SetDirichlet(vector<float> *dirichlet);
        void SetCoefficient(vector<float> *coefficient);

        void GetSolution(vector<float> *result);

        void Step();
        
        MatrixXf                     m_solutions;

        private:

        /*-----------------------------------------------------------------------------
         * Member variables for mesh, forcing and other problem-related attributes
         *-----------------------------------------------------------------------------*/
        SurfaceTopology              *m_surfaceTopology;
        ForcingFunc                  m_forcingFunc;
        NeumannFunc                  m_neumannFunc;
        int                          m_nt;
        float                        m_dt;
        double                       m_tolerance;
        int                          m_maxIterations;
        int                          m_ts;
        int                          m_nMeshPoints;
        
        /*-----------------------------------------------------------------------------
         * Member variables for various matrices and vectors needed for computing
         * FEM solution
         *-----------------------------------------------------------------------------*/
        Vector2f                     m_shapeDerivatives[3];
        SparseMatrix<float>          m_A_full;
        SparseMatrix<float>          m_A_free;
        SparseMatrix<float>          m_B_full;
        SparseMatrix<float>          m_B_free;
        VectorXf                     m_rhs;

        vector<Coord>                m_elementCentres;
        vector<float>                m_determinants;
        vector<float>                m_dirichlet;
        vector<float>                m_coefficient;
        vector<int>                  m_dirichletNodeIndices;

        void AssembleA();
        void AssembleB();
        void AssembleRHS();
        void InitializeShapeDerivs();
        void LocalStiffnessMatrix(const unsigned int *triIndices, Matrix3f *lsm);
    };
}}

