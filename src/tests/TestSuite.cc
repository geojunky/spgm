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
 *       Filename:  TestSuite.c
 *
 *    Description:  
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
#include <minunit.h>

int tests_run = 0;

extern "C" char *test_mem_fixed();
extern "C" char *test_mem_dynamic();
extern "C" char *test_config();
extern "C" char *test_mesh();
extern "C" char *test_surface_topology();
extern "C" char *test_nl_diffusion();
extern "C" char *test_l_diffusion();

static char * all_tests() {
    mu_run_test(test_config);
    mu_run_test(test_mem_fixed);
    mu_run_test(test_mem_dynamic);

    mu_run_test(test_mesh);
    mu_run_test(test_surface_topology);
    mu_run_test(test_l_diffusion);
    mu_run_test(test_nl_diffusion);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

