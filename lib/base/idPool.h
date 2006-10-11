
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_IDPOOL_H
#define EQBASE_IDPOOL_H

#include <eq/base/base.h>

#include <list>

namespace eqBase
{
#   define EQ_ID_INVALID 0xfffffffeu
#   define EQ_ID_ANY     0xffffffffu

    /**
     * A identifier pool.
     * 
     * Manages unique identifiers.
     */
    class IDPool 
    {
    public:
        enum MaxCapacity
        {
            MAX_CAPACITY =  0xfffffff0
        };

        /** 
         * Constructs a new identifier pool.
         *
         * @param initialCapacity the initial capacity of the pool, the
         *                        identifiers from initialCapacity to
         *                        MAX_CAPACITY are considered as allocated.
         */
        IDPool( const uint32_t initialCapacity );

        /** Destructs the identifier pool. */
        ~IDPool();
        
        /** 
         * Generate a new, consecutive block of identifiers. 
         * 
         * @param range The number of identifiers to allocate
         * 
         * @return the first identifier of the block, or EQ_ID_INVALID if no
         *         block of size range is available.
         */
        uint32_t genIDs( const uint32_t range );
        void freeIDs( const uint32_t start, const uint32_t range );


    private:
        struct Block
        {
            uint32_t start;
            uint32_t range;
        };

        std::list<Block*> _freeIDs;
        std::list<Block*> _blockCache;
        
        uint32_t _genIDs( const uint32_t range );
        uint32_t _compressCounter;
        void _compressIDs();

        CHECK_THREAD_DECLARE( _thread );
    };
}

#endif //EQBASE_IDPOOL_H
