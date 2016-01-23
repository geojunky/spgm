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
 *       Filename:  RegularMesh.hh
 *
 *    Description:  Implements a regular cartesian axis-aligned mesh
 *
 *        Version:  1.0
 *        Created:  12/06/14 14:02:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)         
 *
 * =====================================================================================
 */
#ifndef SRC_MESH_REGULARMESH_HH
#define SRC_MESH_REGULARMESH_HH

#include <KdTree.hh>
#include <Timer.hh>
#include <Eigen/Dense>

namespace src { namespace mesh {

    using namespace std;
    using namespace Eigen;

    /*
     * =====================================================================================
     *        Class:  KdTree
     *  Description:  kd-tree definition for spatial analysis
     * =====================================================================================
     */
    class RegularMesh
    {
        public:

        typedef Matrix<double, Dynamic, Dynamic, RowMajor> MatrixRM;

        friend class SurfaceTopology;

        RegularMesh(int nx, int ny, const float *upper, const float *lower);
        ~RegularMesh();
        
        inline double X(int i, int j){ return m_coords[i*m_nx + j][0]; }
        inline double Y(int i, int j){ return m_coords[i*m_nx + j][1]; }
        inline double &V(int i, int j){ return m_values(i,j); }

        void UpdateInterpolator();
        void GetFunctionValuesAt(int nCoor, float **coor, vector<float> *result);
        
        void Print();

        private:

        MatrixRM m_values;
        double **m_coords;

        int m_nx;
        int m_ny;
        double m_upper[2];
        double m_lower[2];
        double m_dx;
        double m_dy;
        
        /* Interpolation related */
        MatrixRM m_ya;
        MatrixRM m_y2a;
        vector<double> m_x1a;
        vector<double> m_x2a;

        void spline( vector<double> &x, vector<double> &y, 
                     const double yp1, const double ypn,
                     vector<double> &y2 );
        void splint( vector<double> &xa, vector<double> &ya, 
                     vector<double> &y2a, const double x, double &y );
        void splie2( vector<double> &x1a, vector<double> &x2a, 
                     MatrixRM &ya, MatrixRM &y2a );   
        void splin2(vector<double> &x1a, vector<double> &x2a, 
                    MatrixRM &ya, MatrixRM &y2a, const double x1, 
                    const double x2, double &y);
    };
}}
#endif

