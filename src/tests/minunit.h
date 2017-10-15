/*
 * =====================================================================================
 *
 *       Filename:  minunit.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14/10/17 21:40:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

 #define mu_assert(message, test) do { if (!(test)) return message; } while (0)
 #define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
 extern int tests_run;
