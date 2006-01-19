
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_IDPOOL_H
#define EQBASE_IDPOOL_H

#include "base.h"

#include <list>

namespace eqBase
{
    /**
     * A identifier pool.
     * 
     * This pool manages unique identifiers. The id '0' is not a valid
     * identifier and therefore only returned upon error.
     */
    class IDPool 
    {
    public:
        /** 
         * Constructs a new identifier pool.
         */
        IDPool();

        /** Destructs the identifier pool. */
        ~IDPool();
        
        uint32_t genIDs( const uint32_t range );
        void freeIDs( const uint32_t start, const uint32_t range );

        static uint32_t getCapacity() { return 0xfffffff0; }

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

#ifdef CHECK_THREADSAFETY
        pthread_t _threadID;
#endif
    };
}

#endif //EQBASE_IDPOOL_H
