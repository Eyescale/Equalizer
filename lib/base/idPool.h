
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQBASE_IDPOOL_H
#define EQBASE_IDPOOL_H

#include <eq/base/base.h>
#include <eq/base/nonCopyable.h>
#include <eq/base/thread.h>

#include <list>

namespace eq
{
namespace base
{
#   define EQ_ID_MAX     0xfffffff0u //!< The biggest identifier possible
#   define EQ_ID_NONE    0xfffffffdu //!< None/NULL identifier
#   define EQ_ID_INVALID 0xfffffffeu //!< Invalid/unset identifier
#   define EQ_ID_ANY     0xffffffffu //!< Any/all identifiers

    /**
     * An identifier pool.
     * 
     * Manages re-usable, unique identifiers. Can allocate up to MAX_CAPACITY
     * identifiers. Used in Equalizer for session-unique object
     * identifiers. Access to the identifier pool is thread-safe.
     */
    class IDPool : public NonCopyable
    {
    public:
        enum MaxCapacity
        {
            MAX_CAPACITY = EQ_ID_MAX //!< The maximum capacity of the pool.
        };

        /** 
         * Construct a new identifier pool.
         *
         * @param initialCapacity the initial capacity of the pool, the
         *                        identifiers from initialCapacity to
         *                        MAX_CAPACITY are considered as allocated.
         * @version 1.0
         */
        IDPool( const uint32_t initialCapacity );

        /** Destruct the identifier pool. */
        ~IDPool();
        
        /** 
         * Generate a new, consecutive block of identifiers. 
         * 
         * @param range The number of identifiers to allocate
         * @return the first identifier of the block, or EQ_ID_INVALID if no
         *         block of the range is available.
         * @version 1.0
         */
        uint32_t genIDs( const uint32_t range );

        /** 
         * Release a block of previously generated identifiers.
         * 
         * @param start the first identifier of the block.
         * @param range the number of consecutive identifiers.
         * @version 1.0
         */
        void freeIDs( const uint32_t start, const uint32_t range );


    private:
        struct Block
        {
            uint32_t start;
            uint32_t range;
        };

        std::list<Block*> _freeIDs;
        std::vector<Block*> _blockCache;

        Lock _lock;

        uint32_t _genIDs( const uint32_t range );
        uint32_t _compressCounter;
        void _compressIDs();
    };
}

}
#endif //EQBASE_IDPOOL_H
