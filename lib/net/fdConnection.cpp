
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef WIN32
#include "fdConnection.h"
#include "log.h"

#include <eq/base/base.h>
#include <errno.h>

#ifndef WIN32
#  include <poll.h>
#endif

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
FDConnection::FDConnection()
        : _readFD( 0 ),
          _writeFD( 0 )
{}

FDConnection::FDConnection( const FDConnection& conn )
        : Connection( conn ),
          _readFD( conn._readFD ),
          _writeFD( conn._writeFD )
{}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
int64_t FDConnection::read( void* buffer, const uint64_t bytes )
{
    if( _readFD < 1 )
        return -1;

    const ssize_t bytesRead = ::read( _readFD, buffer, bytes );

    if( bytesRead == 0 ) // EOF
    {
        EQINFO << "Got EOF, closing connection" << endl;
        close();
        return -1;
    }

    if( bytesRead == -1 ) // error
    {
        if( errno == EINTR ) // if interrupted, try again
            return 0;

        EQWARN << "Error during read: " << strerror( errno ) << ", " << bytes
               << " bytes on fd " << _readFD << endl;
        return -1;
    }

    return bytesRead;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
int64_t FDConnection::write( const void* buffer, const uint64_t bytes ) const
{
    if( _writeFD < 1 )
        return -1;

    const ssize_t bytesWritten = ::write( _writeFD, buffer, bytes );

    if( bytesWritten == -1 ) // error
    {
        if( errno == EINTR ) // if interrupted, try again
            return 0;

        EQWARN << "Error during write: " << strerror( errno ) << endl;
        return -1;
    }

    return bytesWritten;
}

bool FDConnection::hasData() const
{
    pollfd fd;
    fd.events  = POLLIN;
    fd.fd      = getReadNotifier();
    EQASSERT( fd.fd > 0 );

    const int nReady = poll( &fd, 1, 0 );
    return nReady > 0;
}

}
}
#endif // WIN32
