
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

#include "memoryMap.h"

#include "debug.h"
#include "os.h"
#include "types.h"

#include <fcntl.h>
#include <sys/stat.h>
#ifndef _WIN32
#   include <sys/mman.h>
#endif

namespace co
{
namespace base
{

MemoryMap::MemoryMap()
#ifdef _WIN32
        : _map( 0 )
#else
        : _fd( 0 )
#endif
        , _ptr( 0 )
        , _size( 0 )
{}

MemoryMap::~MemoryMap()
{
    if( _ptr )
        unmap();
}

const void* MemoryMap::map( const std::string& filename )
{
    if( _ptr )
    {
        EQWARN << "File already mapped" << std::endl;
        return 0;
    }

#ifdef _WIN32
    // try to open binary file
    HANDLE file = CreateFile( filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                              0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if( file == INVALID_HANDLE_VALUE )
    {
        EQWARN << "Can't open " << filename << ": " << sysError
               << std::endl;
        return 0;
    }

    // create a file mapping
    _map = CreateFileMapping( file, 0, PAGE_READONLY, 0, 0, 0 );
    if( !_map )
    {
        EQWARN << "File mapping failed: " << sysError << std::endl;
        return 0;
    }
    
    // get a view of the mapping
    _ptr = MapViewOfFile( _map, FILE_MAP_READ, 0, 0, 0 );
    
    // get size
    DWORD highSize;
    const DWORD lowSize = GetFileSize( file, &highSize );
    _size = lowSize | ( static_cast< uint64_t >( highSize ) << 32 );

    CloseHandle( file );
#else // POSIX

    // try to open binary file
    _fd = open( filename.c_str(), O_RDONLY );
    if( _fd < 0 )
    {
        EQINFO << "Can't open " << filename << std::endl;
        return 0;
    }
    
    // retrieving file information
    struct stat status;
    fstat( _fd, &status );
    
    // create memory mapped file
    _size = status.st_size;
    _ptr = mmap( 0, _size, PROT_READ, MAP_SHARED, _fd, 0 );

    if( _ptr == MAP_FAILED )
    {
        close( _fd );
        _ptr = 0;
        _size = 0;
        _fd = 0;
    }
#endif

    return _ptr;
}

void MemoryMap::unmap()
{
    if( !_ptr )
    {
        EQWARN << "File not mapped" << std::endl;
        return;
    }

#ifdef _WIN32
    UnmapViewOfFile( _ptr );
    CloseHandle( _map );
    
    _ptr = 0;
    _size = 0;
    _map = 0;
#else
    munmap( _ptr, _size );
    close( _fd );
    
    _ptr = 0;
    _size = 0;
    _fd = 0;
#endif
}

}
}
