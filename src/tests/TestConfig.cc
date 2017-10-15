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
 *       Filename:  TestConfig.cc
 *
 *    Description:  Tests for configuration parser.
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
#include <stdlib.h>
#include <iostream>
#include <minunit.h>

using namespace src::parser;
using namespace std;

extern "C" char *test_config()
{
    cout << "===== Testing Config Parser =====" << endl;

    Config c("src/tests/data/mms.cfg");
    
    mu_assert("Failure: file-name mismatch", c.PString("fileName")=="src/tests/data/mmsMesh.txt");
    mu_assert("Failure: smoothing factor mismatch", c.PDouble("smoothing")==0);
    mu_assert("Failure: smoothing iterations mismatch", c.PInt("smoothingIterations")==500);
    
    cout << "Verified configuration parameters.." << endl;
    cout << "======================================" << endl << endl;
    return 0;
}

