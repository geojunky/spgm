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
 *       Filename:  TestMem.cc
 *
 *    Description:  Tests for memory module.
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

#include <minunit.h>
#include <MemoryPool.hh>
#include <iostream>
#include <vector>

using namespace src::mem;
using namespace std;

class Cplx
{
    float re;
    float im;
};

extern "C" char *test_mem_fixed()
{
    int len = 5000;
    cout << "===== Testing Fixed-sized Pool =====" << endl;

    MemoryPool *p = new MemoryPool( sizeof(Cplx), len, MemoryPool::Fixed );
    
    vector<char*> allocs;
    for(int i=0; i<len; i++)
    {
        char *newObject = (char*)p->NewObject();
        allocs.push_back(newObject);
    }
    
    mu_assert("Failure: Number of allocations mismatch", p->GetNumFree()==0);
    
    for(int i=0; i<len; i++)
        mu_assert("Failure: Deallocation failure", p->DeleteObject(allocs[i]));
    
    
    cout << "Allocated and deallocated " << len << " elements.." << endl;
    delete p;
    cout << "======================================" << endl << endl;
    return 0;
}

extern "C" char *test_mem_dynamic()
{
    int len = 500;
    cout << "===== Testing Dynamic Pool =====" << endl;

    MemoryPool *p = new MemoryPool( sizeof(Cplx), len, MemoryPool::Dynamic);
    
    int totalAlloc = 1001;
    
    vector<char*> allocs;
    for(int i=0; i<totalAlloc; i++)
    {
        char *newObject = (char*)p->NewObject();
        allocs.push_back(newObject);
        mu_assert("Failure: Received NULL pointer", newObject != NULL);
    }
    
    for(int i=0; i<totalAlloc; i++)
    {
        mu_assert("Failure: Deallocation failure", p->DeleteObject(allocs[i]));
    }
    
    cout << "Allocated and deallocated " << totalAlloc << " elements.." << endl;
    delete p;
    cout << "======================================" << endl << endl;
    return 0;
}

