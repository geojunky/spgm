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
 *       Filename:  TestDiffusion.cc
 *
 *    Description:  Tests for linear and nonlinear diffusion, using the method of 
 *                  manufactured solutions.
 *
 *        Version:  1.0
 *        Created:  14/10/17 21:42:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@ga.gov.au)
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <Config.hh>
#include <math.h>
#include <stdlib.h>
#include <SurfaceTopology.hh>
#include <Diffusion.hh>
#include <minunit.h>

using namespace src::parser;
using namespace src::mesh;
using namespace src::math;
using namespace std;

/*-----------------------------------------------------------------------------
 * Functions needed for comparing numerical and analytical numSols
 *-----------------------------------------------------------------------------*/
float source(float x, float y, float t)
{
    return -4*pow(x, 2)*(pow(sin(x), 2) + pow(cos(y), 2) + 1)*exp(-t/5)*exp(-pow(x, 2) - pow(y, 2)) + 4*x*exp(-t/5)*exp(-pow(x, 2) - pow(y, 2))*sin(x)*cos(x) - 4*pow(y, 2)*(pow(sin(x), 2) + pow(cos(y), 2) + 1)*exp(-t/5)*exp(-pow(x, 2) - pow(y, 2)) - 4*y*exp(-t/5)*exp(-pow(x, 2) - pow(y, 2))*sin(y)*cos(y) + 4*(pow(sin(x), 2) + pow(cos(y), 2) + 1)*exp(-t/5)*exp(-pow(x, 2) - pow(y, 2)) - exp(-t/10)*exp(-pow(x, 2) - pow(y, 2))/10;
}

float k(float x, float y, float t)
{
    return (pow(sin(x), 2) + pow(cos(y), 2) + 1)*exp(-t/10);
}

/* Source function for k=1 */
float source_k1(float x, float y, float t)
{
    return -4*pow(x, 2)*exp(-1.0L/10.0L*t)*exp(-pow(x, 2) - pow(y, 2)) - 4*pow(y, 2)*exp(-1.0L/10.0L*t)*exp(-pow(x, 2) - pow(y, 2)) + (39.0L/10.0L)*exp(-1.0L/10.0L*t)*exp(-pow(x, 2) - pow(y, 2));
}

float u_exact(float x, float y, float t)
{
    return exp(-t/10)*exp(-pow(x, 2) - pow(y, 2));
}


extern "C" char *test_nl_diffusion()
{
    cout << "===== Testing Nonlinear Diffusion =====" << endl;
    float t = 0.;
    int   nt = 5;
    float dt = 1e-3;

    Config c("src/tests/data/mms.cfg");
    SurfaceTopology st(&c);

    Diffusion diffusion(&st, source, NULL, nt, dt, 1e-5, 200);
    
    int len = st.GetNMeshPoints();    
    int nelem = st.GetNumTriangles();
    const unsigned int **triangles = st.GetTriangleIndices();
    
    vector<float> z(len);

    for(int i=0; i<len; i++) z[i] = st.Z(i);
    diffusion.SetIC(&z);

    for(int i=0; i<nt; i++)
    {
        t += dt;
        
        vector<float> anaSol(len);        
        vector<float> d(len);
        vector<float> coefficient(len);        
        for(int j=0; j<len; j++)
        {
            if(st.B(j)==SurfaceTopology::DIRICHLET) d[j] = u_exact(st.X(j), st.Y(j), t);
            else d[j] = 0.;

            anaSol[j] = u_exact(st.X(j), st.Y(j), t);
            coefficient[j] = k(st.X(j), st.Y(j), t);
        }
        diffusion.SetDirichlet(&d);

        vector<float> elemCoefficient(nelem);
        vector<float> numSol(len);        
        for(int ie=0; ie<nelem; ie++)
        {
            const unsigned int *triIndices = triangles[ie];

            for(int k=0; k<3; k++)
            {
                elemCoefficient[ie] += coefficient[triIndices[k]];
            }
            elemCoefficient[ie] /= 3.0f;
        }     
        
        diffusion.SetCoefficient(&elemCoefficient);
        diffusion.Step();
        diffusion.GetSolution(&numSol);
        
        for(int j=0; j<len; j++)
        {
            //printf("%f %f %e %e %e\n", st.X(j), st.Y(j), numSol[j], anaSol[j],
            //        fabs(numSol[j]-anaSol[j]));
            mu_assert("Failure: Absolute error > 1e-3", fabs(numSol[j]-anaSol[j]) < 1e-3);
        }
    }
    cout << "Numerical solution within tolerance (1e-3).." << endl;
    cout << "======================================" << endl << endl;
    return 0;
}

extern "C" char *test_l_diffusion()
{
    cout << "===== Testing Linear Diffusion =====" << endl;
    float t = 0.;
    int   nt = 5;
    float dt = 0.001;

    Config c("src/tests/data/mms.cfg");
    SurfaceTopology st(&c);

    Diffusion diffusion(&st, source_k1, NULL, nt, dt, 1e-5, 200);
    
    int len = st.GetNMeshPoints();    
    int nelem = st.GetNumTriangles();
    const unsigned int **triangles = st.GetTriangleIndices();
    
    vector<float> z(len);

    for(int i=0; i<len; i++) z[i] = st.Z(i);
    diffusion.SetIC(&z);

    for(int i=0; i<nt; i++)
    {
        t += dt;
        
        vector<float> anaSol(len);        
        vector<float> d(len);
        vector<float> coefficient(len);        
        for(int j=0; j<len; j++)
        {
            if(st.B(j)==SurfaceTopology::DIRICHLET) d[j] = u_exact(st.X(j), st.Y(j), t);
            else d[j] = 0.;

            anaSol[j] = u_exact(st.X(j), st.Y(j), t);
            coefficient[j] = 1;
        }
        diffusion.SetDirichlet(&d);

        vector<float> elemCoefficient(nelem);
        vector<float> numSol(len);        
        for(int ie=0; ie<nelem; ie++)
        {
            const unsigned int *triIndices = triangles[ie];

            for(int k=0; k<3; k++)
            {
                elemCoefficient[ie] += coefficient[triIndices[k]];
            }
            elemCoefficient[ie] /= 3.0f;
        }     
        
        diffusion.SetCoefficient(&elemCoefficient);
        diffusion.Step();
        diffusion.GetSolution(&numSol);
        
        for(int j=0; j<len; j++)
        {
            //printf("%f %f %e %e %e\n", st.X(j), st.Y(j), numSol[j], anaSol[j],
            //        fabs(numSol[j]-anaSol[j]));
            mu_assert("Failure: Absolute error > 1e-3", fabs(numSol[j]-anaSol[j]) < 1e-3);
        }
    }
    cout << "Numerical solution within tolerance (1e-3).." << endl;
    cout << "======================================" << endl << endl;
    return 0;
}

