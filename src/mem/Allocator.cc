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
 *       Filename:  Allocator.cc
 *
 *    Description:  Allocator
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:56:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#include <Allocator.hh>
#include <pthread.h>

namespace src { namespace mem {

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern int progCount;
extern std::vector<MemoryPool*> pools;

void InitPool()
{
    pthread_mutex_lock( &mutex );

    progCount++;
    
    pthread_mutex_unlock( &mutex );
}

void FinalisePool()
{
    pthread_mutex_lock( &mutex );
    progCount--;

    if( progCount == 0 )
    {
        for( unsigned int i=0; i<pools.size(); i++ )
        {
            delete( pools[i] );
        }
    }
    pthread_mutex_unlock( &mutex );
}

}}

