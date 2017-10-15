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
 *       Filename:  TestMesh.cc
 *
 *    Description:  Tests for mesh loading and triangulation.
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
#include <minunit.h>

using namespace src::parser;
using namespace src::mesh;
using namespace std;

extern "C" char *test_mesh()
{
    cout << "===== Testing Mesh =====" << endl;

    Config c("src/tests/data/mms.cfg");
    SurfaceTopology st(&c);
    
    mu_assert("Failure: Number of points mismatch", st.GetNMeshPoints() == 6400);
    mu_assert("Failure: Number of triangles mismatch", st.GetNumTriangles() == 12482);
    mu_assert("Failure: Number of Voronoi vertices mismatch", st.GetNumVoronoiVertices() == 12482);
    
    cout << "Verified mesh attributes.." << endl;
    cout << "======================================" << endl << endl;
    return 0;
}

extern "C" char *test_surface_topology()
{
    cout << "===== Testing Surface Topology =====" << endl;

    Config c("src/tests/data/mms.cfg");
    SurfaceTopology st(&c);

    SurfaceTopology::CatchmentIterator iterBegin = st.CatchmentsBegin();
    SurfaceTopology::CatchmentIterator iterEnd   = st.CatchmentsEnd();
    
    int ncatch = distance(iterBegin, iterEnd);

    mu_assert("Failure: Number of catchments mismatch", ncatch == 316);

    cout << "Verified number of catchments.." << endl;
    cout << "======================================" << endl << endl;
    return 0;
}

