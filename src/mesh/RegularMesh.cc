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
 *       Filename:  RegularMesh.cc
 *
 *    Description:  Implementation of RegularMesh
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

#include <RegularMesh.hh>
#include <math.h>
#include <assert.h>
namespace src { namespace mesh {
using namespace std;
    RegularMesh::RegularMesh(int nx, int ny, const float *upper, const float *lower)
    :m_nx(nx),
    m_ny(ny)
    {
        
        /*-----------------------------------------------------------------------------
         * Sanity check
         *-----------------------------------------------------------------------------*/
        assert(upper[0]>lower[0]);
        assert(upper[1]>lower[1]);
        assert(nx>1);
        assert(ny>1);
        
        for(int i=0; i<2; i++)
        {
            m_upper[i] = upper[i];
            m_lower[i] = lower[i];
        }

        m_dx = (m_upper[0]-m_lower[0]) / (m_nx - 1);
        m_dy = (m_upper[1]-m_lower[1]) / (m_ny - 1);

        /*-----------------------------------------------------------------------------
         * Resize matrices (matrices are stored in column-major format by default)
         * Allocate memory for coordinates
         *-----------------------------------------------------------------------------*/
        m_values.resize(m_nx, m_ny);
        m_coords = new double*[m_nx*m_ny];
        m_coords[0] = new double[m_nx*m_ny*2];
        for(int i=0; i<m_nx*m_ny; i++) m_coords[i] = m_coords[0] + i*2;

        for(int i=0; i<m_nx; i++)
        {
            for(int j=0; j<m_ny; j++)
            {
                m_coords[i*m_nx+j][0]   = i*m_dx;
                m_coords[i*m_nx+j][1]   = j*m_dy;
                m_values(i,j)           = 0;
            }
        }

        /*-----------------------------------------------------------------------------
         * Initialize m_x1a and m_x2a 
         *-----------------------------------------------------------------------------*/
        m_x1a.resize(m_nx);
        m_x2a.resize(m_ny);
        for(int i=0; i<m_nx; i++) m_x1a[i] = i*m_dx;
        for(int j=0; j<m_ny; j++) m_x2a[j] = j*m_dy;
    }

    RegularMesh::~RegularMesh()
    {
        delete [] m_coords[0];
        delete [] m_coords;
    }
    
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  RegularMesh
     *      Method:  RegularMesh :: Print
     * Description:  
     *--------------------------------------------------------------------------------------
     */
    void RegularMesh::Print()
    {
        for(int i=0; i<m_nx; i++)
        {
            for(int j=0; j<m_ny; j++)
            {
                printf("%f %f %f\n", m_coords[i*m_nx+j][0], m_coords[i*m_nx+j][1],
                        m_values(i,j));
            }
        }
    }
    
    /*
     *--------------------------------------------------------------------------------------
     *       Class:  RegularMesh
     *      Method:  RegularMesh :: UpdateInterpolator
     * Description:  Update m_ya and m_y2a 
     *--------------------------------------------------------------------------------------
     */
    void RegularMesh::UpdateInterpolator()
    {
        m_y2a.resize(m_nx, m_ny);
        m_y2a.setZero();

        splie2(m_x1a, m_x2a, m_values, m_y2a);
    }
    
    void RegularMesh::GetFunctionValuesAt(int nCoor, float **coor, vector<float> *result)
    {
        UpdateInterpolator();
        
        result->resize(nCoor);
        
        #pragma omp parallel for
        for(int i=0; i<nCoor; i++)
        {
            double res = 0;
            splin2(m_x1a, m_x2a, m_values, m_y2a, coor[i][0], coor[i][1], res);
            (*result)[i] = res;
        }
    }

    /*-----------------------------------------------------------------------------
     * Private functions
     *-----------------------------------------------------------------------------*/
    void RegularMesh::spline( vector<double> &x, vector<double> &y, 
                              const double yp1, const double ypn,
                              vector<double> &y2 )
    {
        int i,k;
        double p,qn,sig,un;

        int n=y2.size();
        vector<double> u(n-1);
        if (yp1 > 0.99e30)
            y2[0]=u[0]=0.0;
        else {
            y2[0] = -0.5;
            u[0]=(3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
        }
        for (i=1;i<n-1;i++) {
            sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
            p=sig*y2[i-1]+2.0;
            y2[i]=(sig-1.0)/p;
            u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
            u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
        }
        if (ypn > 0.99e30)
            qn=un=0.0;
        else {
            qn=0.5;
            un=(3.0/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
        }
        y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0);
        for (k=n-2;k>=0;k--)
            y2[k]=y2[k]*y2[k+1]+u[k];
    }
    
    void RegularMesh::splint( vector<double> &xa, vector<double> &ya, 
                             vector<double> &y2a, const double x, double &y )
    {
        int k;
        double h,b,a;

        int n=xa.size();
        int klo=0;
        int khi=n-1;
        while (khi-klo > 1) {
            k=(khi+klo) >> 1;
            if (xa[k] > x) khi=k;
            else klo=k;
        }
        h=xa[khi]-xa[klo];
        if (h == 0.0) assert("Bad xa input to routine splint");
        a=(xa[khi]-x)/h;
        b=(x-xa[klo])/h;
        y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]
            +(b*b*b-b)*y2a[khi])*(h*h)/6.0;
    }

    void RegularMesh::splie2(vector<double> &x1a, vector<double> &x2a, 
                             MatrixRM &ya, MatrixRM &y2a)
    {
        int m,n,j,k;

        m=x1a.size();
        n=x2a.size();
        vector<double> ya_t(n),y2a_t(n);
        for (j=0;j<m;j++) {
            for (k=0;k<n;k++) ya_t[k]=ya(j,k);
            spline(x2a,ya_t,1.0e30,1.0e30,y2a_t);
            for (k=0;k<n;k++) y2a(j,k)=y2a_t[k];
        }
    }

    void RegularMesh::splin2(vector<double> &x1a, vector<double> &x2a, 
                             MatrixRM &ya, MatrixRM &y2a, const double x1, 
                             const double x2, double &y)
    {
        int j,k;

        int m=x1a.size();
        int n=x2a.size();
        vector<double> ya_t(n),y2a_t(n),yytmp(m),ytmp(m);
        for (j=0;j<m;j++) {
            for (k=0;k<n;k++) {
                ya_t[k]=ya(j,k);
                y2a_t[k]=y2a(j,k);
            }
            splint(x2a,ya_t,y2a_t,x2,yytmp[j]);
        }
        spline(x1a,yytmp,1.0e30,1.0e30,ytmp);
        splint(x1a,yytmp,ytmp,x1,y);
    }
}}
