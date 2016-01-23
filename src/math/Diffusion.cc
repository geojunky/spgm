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
 *       Filename:  Diffusion.cc
 *
 *    Description:  
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

#include <Diffusion.hh>
#include <assert.h>

namespace src { namespace math {
    using namespace std;
    using namespace src::util;
    using namespace Eigen;

    const int PREV = 0;
    const int CURR = 1;

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: Diffusion
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    Diffusion::Diffusion( SurfaceTopology *st, ForcingFunc f, 
                          NeumannFunc n, int nt, float dt, 
                          double tolerance, int maxIterations ):
    m_surfaceTopology(st),
    m_forcingFunc(f),
    m_neumannFunc(n),
    m_nt(nt+1),
    m_dt(dt),
    m_tolerance(tolerance),
    m_maxIterations(maxIterations),
    m_ts(1)
    {
        int numTriangles                    = m_surfaceTopology->GetNumTriangles();

        /*-----------------------------------------------------------------------------
         * Initialize various matrices
         *-----------------------------------------------------------------------------*/
        m_nMeshPoints                            = m_surfaceTopology->GetNMeshPoints();
        m_rhs.resize(m_nMeshPoints);             m_rhs.fill(0);
        m_solutions.resize(m_nMeshPoints, 2); m_solutions.fill(0);

        InitializeShapeDerivs();

        /*-----------------------------------------------------------------------------
         * Initialize vectors
         *-----------------------------------------------------------------------------*/
        m_dirichlet.reserve(m_nMeshPoints);
        m_coefficient.reserve(numTriangles);

        /*-----------------------------------------------------------------------------
         * Initialize sparse matrices
         *-----------------------------------------------------------------------------*/
        m_A_full = SparseMatrix<float>(m_nMeshPoints, m_nMeshPoints);
        m_A_free = SparseMatrix<float>(m_nMeshPoints, m_nMeshPoints);
        m_B_full = SparseMatrix<float>(m_nMeshPoints, m_nMeshPoints);
        m_B_free = SparseMatrix<float>(m_nMeshPoints, m_nMeshPoints);    
        
        /*-----------------------------------------------------------------------------
         * Precompute element centres and determinants
         *-----------------------------------------------------------------------------*/
        Matrix2f B;
        const unsigned int **triIndices     = m_surfaceTopology->GetTriangleIndices();
        
        m_elementCentres.reserve(numTriangles);
        m_determinants.reserve(numTriangles);
        for(int ie=0; ie<numTriangles; ie++)
        {
            const unsigned int *elemTriIndices = triIndices[ie];
            int v0 = elemTriIndices[0];
            int v1 = elemTriIndices[1];
            int v2 = elemTriIndices[2];
            
            float cx = ( m_surfaceTopology->X(v0) + 
                         m_surfaceTopology->X(v1) + 
                         m_surfaceTopology->X(v2) ) / 3.0f;
            float cy = ( m_surfaceTopology->Y(v0) + 
                         m_surfaceTopology->Y(v1) + 
                         m_surfaceTopology->Y(v2) ) / 3.0f;
            m_elementCentres[ie] = Coord(cx, cy);

            B << ( st->X(v0)-st->X(v2) ), ( st->X(v1)-st->X(v2) ),
                 ( st->Y(v0)-st->Y(v2) ), ( st->Y(v1)-st->Y(v2) );

            m_determinants[ie] = B.determinant();
            assert(m_determinants[ie] >= 0.);
        }

        /*-----------------------------------------------------------------------------
         * Store indices of Dirichlet and Free nodes
         *-----------------------------------------------------------------------------*/
        for(int in=0; in<m_nMeshPoints; in++)
        {
            if(((int)st->B(in))==SurfaceTopology::DIRICHLET)
            {
                m_dirichletNodeIndices.push_back(in);
            }
        }
        
        /*-----------------------------------------------------------------------------
         * Assemble matrix B, which does not change
         *-----------------------------------------------------------------------------*/
        AssembleB();
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: ~Diffusion
     * Description:  
     *--------------------------------------------------------------------------------------
     */
    Diffusion::~Diffusion()
    {
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: initializeShapeDerivs
     * Description:  
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::InitializeShapeDerivs()
    {
        m_shapeDerivatives[0] = Vector2f(  1, 0 );
        m_shapeDerivatives[1] = Vector2f(  0, 1 );
        m_shapeDerivatives[2] = Vector2f( -1,-1 );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: localStiffnessMatrix
     * Description:  Computes the local stiffness matrix for an element. 'lsm' is the 3x3 
     *               matrix to which the results are written.
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::LocalStiffnessMatrix(const unsigned int *triIndices, Matrix3f *lsm)
    {
        Matrix2f B, B_inv;
        float determinant;
        bool invertible;
        int v0 = triIndices[0];
        int v1 = triIndices[1];
        int v2 = triIndices[2];
        
        SurfaceTopology *st = m_surfaceTopology;
        B << ( st->X(v0)-st->X(v2) ), ( st->Y(v0)-st->Y(v2) ),
             ( st->X(v1)-st->X(v2) ), ( st->Y(v1)-st->Y(v2) );
        B.computeInverseAndDetWithCheck(B_inv,determinant,invertible);

        if(!invertible)
        {
            cerr << "Error: uninvertible local stiffness matrix encountered. Aborting.." << endl;
            exit(EXIT_FAILURE);
        }

        for(int i=0; i<3; i++)
        {
            for(int j=0; j<3; j++)
            {
                (*lsm)(i,j) = 0.5 * determinant * 
                              (B_inv * m_shapeDerivatives[i]).transpose() * 
                              (B_inv * m_shapeDerivatives[j]);
            }
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: assembleA 
     * Description:  Assemble global stiffness matrix
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::AssembleA()
    {
        const unsigned int **triIndices     = m_surfaceTopology->GetTriangleIndices();
        int numTriangles                    = m_surfaceTopology->GetNumTriangles();
        
        Matrix3f lsm;
        vector< Triplet<float> > triplets;
        for(int ie=0; ie<numTriangles; ie++)
        {
            const unsigned int *elemTriIndices = triIndices[ie];

            LocalStiffnessMatrix(elemTriIndices, &lsm);
            
            lsm *= m_coefficient[ie];
            for(int i=0; i<3; i++)
            {
                for(int j=0; j<3; j++)
                {
                    Triplet<float> t(elemTriIndices[i], elemTriIndices[j], lsm(i,j));
                    triplets.push_back(t);
                }
            }
        }
        m_A_full.setFromTriplets(triplets.begin(), triplets.end());
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: AssembleB
     * Description:  Assemble global mass matrix
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::AssembleB()
    {
        Matrix3f lmm;
        Matrix2f B;
        const unsigned int **triIndices     = m_surfaceTopology->GetTriangleIndices();
        int numTriangles                    = m_surfaceTopology->GetNumTriangles();
        vector< Triplet<float> > triplets;

        lmm <<  2, 1, 1,
                1, 2, 1,
                1, 1, 2;
        
        for(int ie=0; ie<numTriangles; ie++)
        {
            const unsigned int *elemTriIndices = triIndices[ie];

            for(int i=0; i<3; i++)
            {
                for(int j=0; j<3; j++)
                {
                    Triplet<float> t( elemTriIndices[i], 
                                      elemTriIndices[j], 
                                      m_determinants[ie] * lmm(i,j)/24.0f );
                    triplets.push_back(t);
                }
            }            
        }
        m_B_full.setFromTriplets(triplets.begin(), triplets.end());
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: AssembleRHS
     * Description:  Assemble RHS
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::AssembleRHS()
    {
        /*-----------------------------------------------------------------------------
         * Reset RHS
         *-----------------------------------------------------------------------------*/
        m_rhs.fill(0);

        /*-----------------------------------------------------------------------------
         * Volume forces
         *-----------------------------------------------------------------------------*/
        if(m_forcingFunc)
        {
            const unsigned int **triIndices     = m_surfaceTopology->GetTriangleIndices();
            int numTriangles                    = m_surfaceTopology->GetNumTriangles();
            for(int ie=0; ie<numTriangles; ie++)
            {
                const unsigned int *elemTriIndices = triIndices[ie];

                for(int i=0; i<3; i++)
                {
                    m_rhs(elemTriIndices[i]) += 1./6. * m_determinants[ie] * 
                                                m_forcingFunc( m_elementCentres[ie].x, 
                                                           m_elementCentres[ie].y, 
                                                           m_dt*m_ts ) * 
                                                m_dt;
                }
            }
        }

        /*-----------------------------------------------------------------------------
         * Neumann conditions
         *-----------------------------------------------------------------------------*/
        /* TODO */

        
        /*-----------------------------------------------------------------------------
         * So far we've computed what is 'b*dt' for a Poisson equation; we now need to
         * compute b*dt + B*u_n-1
         *-----------------------------------------------------------------------------*/
        VectorXf U_n_1(m_nMeshPoints);

        U_n_1.fill(0);
        for(int i=0; i<m_nMeshPoints; i++) U_n_1(i) = m_solutions(i, PREV);
        m_rhs += m_B_full * U_n_1;

        /*-----------------------------------------------------------------------------
         * Apply Dirichlet conditions
         *-----------------------------------------------------------------------------*/
        VectorXf U_d(m_nMeshPoints);

        U_d.fill(0);
        for( vector<int>::iterator it=m_dirichletNodeIndices.begin(); 
             it != m_dirichletNodeIndices.end(); it++)
        {
            float dVal              = m_dirichlet[*it];
            
            U_d(*it)                = dVal;
            m_solutions(*it, CURR)  = dVal;      
        }
        m_rhs -= (m_dt * m_A_full + m_B_full) * U_d;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: SetIC
     * Description:  Sets the solution for the previous time-step that will affect the 
     *               the solution for the current time-step
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::SetIC(vector<float> *vals)
    {
        for(int i=0; i<m_nMeshPoints; i++) m_solutions(i, PREV) = (*vals)[i];
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: SetDirichlet
     * Description:  Sets the dirichlet node values
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::SetDirichlet(vector<float> *dirichlet)
    {
        for(int i=0; i<m_nMeshPoints; i++) m_dirichlet[i] = (*dirichlet)[i];
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: SetCoefficient
     * Description:  Sets the coefficients for A
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::SetCoefficient(vector<float> *coefficient)
    {
        int numTriangles = m_surfaceTopology->GetNumTriangles();
        for(int i=0; i<numTriangles; i++) m_coefficient[i] = (*coefficient)[i];
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: GetSolution
     * Description:  Returns the solution for the current time-step
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::GetSolution(vector<float> *result)
    {
        for(int i=0; i<m_nMeshPoints; i++) (*result)[i] = m_solutions(i, CURR);
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  Diffusion
     *      Method:  Diffusion :: Step
     * Description:  Proceed to the next time-step
     *--------------------------------------------------------------------------------------
     */
    void Diffusion::Step()
    {
        if(m_ts < m_nt)
        {
            
            ConjugateGradient<SparseMatrix<float> > cg;
            int nFreeNodes = m_nMeshPoints - m_dirichletNodeIndices.size();

            AssembleA();
            AssembleRHS();
            SparseMatrix<float> lhs ( m_dt * m_A_full.topLeftCorner(nFreeNodes, nFreeNodes) + 
                                      m_B_full.topLeftCorner(nFreeNodes, nFreeNodes) );
            
            /*-----------------------------------------------------------------------------
             * Solve linear system 
             *-----------------------------------------------------------------------------*/
            cg.setMaxIterations(m_maxIterations);
            cg.setTolerance(m_tolerance);
            cg.compute(lhs);
            VectorXf sol = cg.solve(m_rhs.head(nFreeNodes));
            
            /*VectorXf sol(m_nMeshPoints);
            for(int i=0; i<m_nMeshPoints; i++) sol(i) = m_solutions(i, PREV);
            {
                int i = 0;
                do 
                {
                    sol = cg.solveWithGuess(m_rhs.head(nFreeNodes), sol);
                    std::cout << i << " : " << cg.error() << std::endl;
                    ++i;
                } while (cg.info()!=Success && i<5000);
            }*/

            /*SparseMatrix<float, ColMajor> A(lhs);
            VectorXf sol;
            SparseLU<SparseMatrix<float, ColMajor>, COLAMDOrdering<int> > solver;
            solver.analyzePattern(A);
            solver.factorize(A);
            sol = solver.solve(m_rhs.head(nFreeNodes));*/

            /*-----------------------------------------------------------------------------
             * Store solution
             *-----------------------------------------------------------------------------*/
            for(int i=0; i<nFreeNodes; i++) m_solutions(i, CURR) = sol(i);
            
            /*-----------------------------------------------------------------------------
             * Increment time-step 
             *-----------------------------------------------------------------------------*/
            m_ts++;

            /*-----------------------------------------------------------------------------
             * Print solver output 
             *-----------------------------------------------------------------------------*/
            cout << "\tDiffusion Solver Iterations: (" << cg.iterations() << ") ";
            cout << ", estimated error: (" << cg.error() << ")" << endl;
            if(cg.iterations() >= m_maxIterations) cout << "\t Warning: solver not converging.." << endl;
        }
        else
        {
            cerr << "Error: overstepping beyond temporal bounds.." << endl;
            exit(EXIT_FAILURE);
        }
    }
}}

