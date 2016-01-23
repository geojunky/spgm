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
 *       Filename:  MemoryPool.hh
 *
 *    Description:  Memory pool for allocating large number of small objects
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:49:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef SRC_MEM_MEMORYPOOL_HH
#define SRC_MEM_MEMORYPOOL_HH

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

namespace src { namespace mem {

const int CHUNK_ARRAY_DELTA = 10;
const int INVALID = -1;
const int SUCCESS = 1;
const int FAILURE = 0;
const int TWO_EXP16 = 65535;

/*
 * =====================================================================================
 *        Class:  Chunk
 *  Description:  A chunk of memory with a free-list
 * =====================================================================================
 */
struct Chunk
{
    char *memory;
    int numFree;
    int chunkId;
    char **freeList;
};

/*
 * =====================================================================================
 *        Class:  MemoryPool
 *  Description:  Memory pool
 * =====================================================================================
 */
class MemoryPool
{
public:
    typedef enum MemoryPoolMode_t
    {
        Fixed,
        Dynamic
    }MemoryPoolMode;

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: MemoryPool
     * Description:  Constructor
     *--------------------------------------------------------------------------------------
     */
    MemoryPool( int elemSize, int numElemsPerChunk, MemoryPoolMode m=Dynamic )
    {
        int i = 0;

        assert( numElemsPerChunk > 0 );
        assert( elemSize > 0 );

        numElementsPerChunk = numElemsPerChunk;

        numChunks = 0;
        elementSize = elemSize;
        mode = m;

        if(mode == Dynamic)
        {
            if( numElementsPerChunk > TWO_EXP16 )
            {
                numElementsPerChunk = TWO_EXP16;
            }
        }

        maxChunkEntries = CHUNK_ARRAY_DELTA;
        chunkToUse = INVALID;

        chunks = ( Chunk* )NULL;
        chunks = ( Chunk* )malloc( sizeof( Chunk ) * maxChunkEntries );
        memset( chunks, 0, sizeof( Chunk )*maxChunkEntries );

        for( i=0; i<maxChunkEntries; i++ )
        {
            chunks[i].chunkId = INVALID;
            chunks[i].numFree = INVALID;
        }

        assert( chunks != NULL );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: GetNumFree
     * Description:  Returns the number of free objects for fixed-sized pools
     *--------------------------------------------------------------------------------------
     */
    unsigned long GetNumFree()
    {
        unsigned long numFree = 0;
        
        if(mode == Fixed)
        {
            int i = 0;
            for( i=0; i<maxChunkEntries; i++ )
            {
                if( chunks[i].numFree != INVALID )
                {
                    numFree += chunks[i].numFree;
                }
            }
        }
        
        return numFree;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: GetDataPointer
     * Description:  Returns the data-pointer for fixed sized pools.
     *--------------------------------------------------------------------------------------
     */
    void *GetDataPointer()
    {
        if(mode==Fixed)
        {
            if(chunks[0].numFree != INVALID)
                return chunks[0].memory;
        }

        return NULL;
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: ~MemoryPool
     * Description:  Destructor
     *--------------------------------------------------------------------------------------
     */
    ~MemoryPool()
    {
        int i = 0;

        for( i=0; i<maxChunkEntries; i++ )
        {
            if( chunks[i].numFree != INVALID )
            {
                free( chunks[i].memory );
                free( chunks[i].freeList );
            }
        }

        free( chunks );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: NewObject
     * Description:  Returns a pointer to a new object
     *--------------------------------------------------------------------------------------
     */
    void* NewObject()
    {
        int             index       = 0;
        Chunk           *chunk      = NULL;

        if( chunkToUse == INVALID )
        {
            chunkToUse = CreateChunk( 0 );
        }

        chunk = &( chunks[chunkToUse] );

        assert( chunk != NULL );
        
        /*-----------------------------------------------------------------------------
         * Abort if this is a fixed-pool and all slots have been allotted
         *-----------------------------------------------------------------------------*/
        if(mode==Fixed)
        {
            if(chunk->numFree <= 0) return NULL;
        }
label:
        index = chunk->numFree - 1;
        if( index < 0 )
        {

            chunkToUse = GetChunkWithFreeSlots();

            if( chunkToUse == INVALID )
            {
                int chunkSlot = GetFreeChunkSlot();

                if( chunkSlot==INVALID )
                {
                    chunkToUse = CreateChunk( maxChunkEntries );
                    assert( chunkToUse != INVALID );
                }
                else
                {
                    chunkToUse = CreateChunk( chunkSlot );
                    assert( chunkToUse != INVALID );
                }
            }

            chunk = &( chunks[chunkToUse] );
            goto label;
        }

        return ( void* )( chunk->freeList[--chunk->numFree] );
    }

    /*
     *--------------------------------------------------------------------------------------
     *       Class:  MemoryPool
     *      Method:  MemoryPool :: DeleteObject
     * Description:  Deallocated an object and takes care of related accounting.
     *--------------------------------------------------------------------------------------
     */
    int DeleteObject( void *object )
    {
        if( object != NULL )
        {
            int                 i           = 0;
            int                 valid       = 0;
            int                 chunkIdx    = 0;

            for( i=0; i<maxChunkEntries; i++ )
            {

                if( chunks[i].memory != NULL &&
                    (( char* )object >= chunks[i].memory ) &&
                    (( char* )object < ( chunks[i].memory+( numElementsPerChunk*elementSize ) ) ) )
                {
                    valid = 1;
                    chunkIdx = i;
                    break;
                }
            }

            if( valid )
            {
                memset( object, 0, elementSize );
                chunks[chunkIdx].freeList[chunks[chunkIdx].numFree++] = ( char* )object;

                Shrink();

                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

private:
    struct Chunk           *chunks;
    size_t                 elementSize;
    int                    numElementsPerChunk;
    int                    numChunks;
    int                    maxChunkEntries;
    int                    chunkToUse;
    MemoryPoolMode         mode;

    int GetChunkWithFreeSlots()
    {
        int i = 0;
        int leastNumFree = INT_MAX;
        int leastNumFreeIdx = INVALID;

        for( i=0; i<maxChunkEntries; i++ )
        {
            if( chunks[i].numFree > 0 )
            {
                if( chunks[i].numFree < leastNumFree )
                {
                    leastNumFree = chunks[i].numFree;
                    leastNumFreeIdx = i;
                }
            }
        }

        return leastNumFreeIdx;
    }

    int GetFreeChunkSlot()
    {
        int i = 0;

        for( i=0; i<maxChunkEntries; i++ )
        {
            if( chunks[i].numFree == INVALID )
            {
                return i;
            }
        }

        return INVALID;
    }

    int CreateChunk( int pos )
    {
        if(( pos ) < maxChunkEntries )
        {
        }
        else
        {
            int  i = 0;

            maxChunkEntries += CHUNK_ARRAY_DELTA;
            chunks = ( Chunk* )realloc( chunks, sizeof( Chunk )*maxChunkEntries );

            assert( chunks != NULL );
            memset( &chunks[maxChunkEntries-CHUNK_ARRAY_DELTA], 0, sizeof( Chunk )*CHUNK_ARRAY_DELTA );

            for( i=( maxChunkEntries-CHUNK_ARRAY_DELTA ); i<maxChunkEntries; i++ )
            {
                chunks[i].chunkId = INVALID;
                chunks[i].numFree = INVALID;
            }
        }

        {
            int        idx     = 0;
            unsigned int i     = 0;
            int        j       = 0;

            idx = pos;

            chunks[idx].memory = ( char* )NULL;
            chunks[idx].memory = ( char* )malloc( sizeof( char ) * elementSize * numElementsPerChunk );
            memset( chunks[idx].memory, 0, sizeof( char )*elementSize * numElementsPerChunk );

            assert( chunks[idx].memory != NULL );

            chunks[idx].chunkId = idx;

            chunks[idx].freeList = ( char** )NULL;
            chunks[idx].freeList = ( char** )malloc( sizeof( char* ) * numElementsPerChunk );

            assert( chunks[idx].freeList != NULL );

            chunks[idx].numFree = numElementsPerChunk;

            for( i=0,j=0; i<numElementsPerChunk*elementSize; i+=elementSize, j++ )
            {
                chunks[idx].freeList[j] = &( chunks[idx].memory[i] );
            }

            ++numChunks;

            return idx;
        }
    }

    void Shrink()
    {
        int                     i                = 0;
        int                     deleteFlag       = 0;
        int                     chunkIdx         = 0;
        int                     shrinkChunkArray = 0;

        for( i=0; i<maxChunkEntries; i++ )
        {
            if( chunks[i].numFree == numElementsPerChunk )
            {
                deleteFlag = 1;
                chunkIdx = i;
                break;
            }
        }

        if( deleteFlag )
        {
            Chunk *c = ( Chunk* )NULL;


            c = &( chunks[chunkIdx] );

            free( c->freeList );
            free( c->memory );
            memset( c, 0, sizeof( Chunk ) );
            c->chunkId = INVALID;
            c->numFree = INVALID;

            numChunks--;

#if 1
label:
            shrinkChunkArray = 1;
            for( i=maxChunkEntries-1; i>=( maxChunkEntries-CHUNK_ARRAY_DELTA ); i-- )
            {
                if( chunks[i].numFree != INVALID )
                {
                    shrinkChunkArray = 0;
                    break;
                }
                else
                {

                }
            }

            if( shrinkChunkArray && maxChunkEntries>CHUNK_ARRAY_DELTA )
            {
                maxChunkEntries-=CHUNK_ARRAY_DELTA;

                if( numChunks > maxChunkEntries )
                {
                    assert( 0 );
                }

                chunks = ( Chunk* )realloc( chunks, sizeof( Chunk )*( maxChunkEntries ) );

                chunkToUse = GetChunkWithFreeSlots();

                goto label;

            }
#endif
        }
    }
};

}}
#endif
