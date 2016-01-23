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
 *       Filename:  Allocator.hh
 *
 *    Description:  Allocator based on memory-pool
 *
 *        Version:  1.0
 *        Created:  24/02/14 15:55:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Rakib Hassan (rakib.hassan@sydney.edu.au)          
 *
 * =====================================================================================
 */
#ifndef PROPAT_MEM_ALLOCATOR_HH
#define PROPAT_MEM_ALLOCATOR_HH

#include <iostream>
#include <vector>

#include <MemoryPool.hh>

namespace src { namespace mem {

extern pthread_mutex_t mutex;
static std::vector<MemoryPool*> pools;

/*
 * =====================================================================================
 *        Class:  PoolAllocator
 *  Description:  PoolAllocator
 * =====================================================================================
 */
template <class T> class PoolAllocator
{
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U>
    struct rebind
    {
        typedef PoolAllocator<U> other;
    };

    static MemoryPool *pool;

    PoolAllocator()
    {
    }

    PoolAllocator( const PoolAllocator& )
    {
    }

    template <class U>
    PoolAllocator( const PoolAllocator<U>& )
    {
    }

    ~PoolAllocator()
    {
    }

    pointer address( reference x ) const
    {
        return &x;
    }
    const_pointer address( const_reference x ) const
    {
        return x;
    }

    pointer allocate( size_type n, const_pointer = 0 )
    {
        pthread_mutex_lock( &mutex );

        if( !PoolAllocator::pool )
        {
            PoolAllocator::pool = new MemoryPool( sizeof( T ), TWO_EXP16 );

            pools.push_back( PoolAllocator::pool );
        }

        void* p = pool->NewObject();
        if( !p )
            throw std::bad_alloc();
        
        pthread_mutex_unlock( &mutex );
        
        return static_cast<pointer>( p );
    }

    void deallocate( pointer p, size_type )
    {
    }

    size_type max_size() const
    {
        return static_cast<size_type>( -1 ) / sizeof( T );
    }

    void construct( pointer p, const value_type& x )
    {
        new( p ) value_type( x );
    }

    void destroy( pointer p )
    {
        //p->~value_type();
    }

private:
    void operator=( const PoolAllocator& );
};

template<> class PoolAllocator<void>
{
    typedef void        value_type;
    typedef void*       pointer;
    typedef const void* const_pointer;

    template <class U>
    struct rebind
    {
        typedef PoolAllocator<U> other;
    };
};


template <class T>
inline bool operator==( const PoolAllocator<T>&,
                        const PoolAllocator<T>& )
{
    return true;
}

template <class T>
inline bool operator!=( const PoolAllocator<T>&,
                        const PoolAllocator<T>& )
{
    return false;
}

template <class T> MemoryPool *PoolAllocator<T>::pool = NULL;

void InitPool();
void FinalisePool();

}}
#endif
