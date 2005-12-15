
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
        
        uint genIDs( const uint range );
        void freeIDs( const uint start, const uint range );

        uint getCapacity() const { return 0xfffffff0; }

    private:
        struct Block
        {
            uint start;
            uint range;
        };

        std::list<Block*> _freeIDs;
        std::list<Block*> _blockCache;
        
        uint _genIDs( const uint range );
        uint _compressCounter;
        void _compressIDs();
    };
}

#endif //EQBASE_IDPOOL_H
