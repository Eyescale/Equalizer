
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_MEMORYMAP_H
#define COBASE_MEMORYMAP_H

#include <co/base/api.h>
#include <co/base/nonCopyable.h>

#include <string>

namespace co
{
namespace base
{
    /** Helper to map a file to a memory address (mmap) */
    class MemoryMap : public NonCopyable
    {
    public:
        /** Construct a new memory map. @version 1.0 */
        COBASE_API MemoryMap();

        /** 
         * Destruct the memory map.
         *
         * Unmaps the file, if it is still mapped.
         * @sa unmap()
         * @version 1.0
         */
        COBASE_API ~MemoryMap();

        /** 
         * Map a file to a memory address.
         *
         * Currently the file is only mapped read-only. The file is
         * automatically unmapped when the memory map is deleted.
         *
         * @param fileName The filename of the file to map.
         * @return the pointer to the mapped file, or 0 upon error.
         * @version 1.0
         */
        COBASE_API const void* map( const std::string& fileName );

        /** Unmap the file. @version 1.0 */
        COBASE_API void unmap();

        /** @return the pointer to the memory map. @version 1.0 */
        const void* getAddress() const { return _ptr; }

        /** @return the size of the memory map. @version 1.0 */
        size_t getSize() const { return _size; }

    private:
#ifdef _WIN32
        void* _map;
#else
        int _fd;
#endif

        void* _ptr;
        size_t _size;
    };

}
}

#endif //COBASE_MEMORYMAP_H
