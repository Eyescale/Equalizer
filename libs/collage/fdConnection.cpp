
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef _WIN32

#include "fdConnection.h"
#include "log.h"

#include <co/base/os.h>

#include <errno.h>
#include <poll.h>

using namespace co::base;

namespace co
{
FDConnection::FDConnection()
        : _readFD( 0 ),
          _writeFD( 0 )
{}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
int64_t FDConnection::readSync( void* buffer, const uint64_t bytes, const bool )
{
    if( _readFD < 1 )
        return -1;

    const ssize_t bytesRead = ::read( _readFD, buffer, bytes );

    if( bytesRead == 0 ) // EOF
    {
        EQINFO << "Got EOF, closing connection" << std::endl;
        close();
        return -1;
    }

    if( bytesRead == -1 ) // error
    {
        if( errno == EINTR ) // if interrupted, try again
            return 0;

        EQWARN << "Error during read: " << strerror( errno ) << ", " 
               << bytes << "b on fd " << _readFD << std::endl;
        return -1;
    }

    return bytesRead;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
int64_t FDConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED || _writeFD < 1 )
        return -1;

    const ssize_t bytesWritten = ::write( _writeFD, buffer, bytes );

    if( bytesWritten == -1 ) // error
    {
        if( errno == EINTR ) // if interrupted, try again
            return 0;

        EQWARN << "Error during write: " << strerror( errno ) << std::endl;
        return -1;
    }

    return bytesWritten;
}

bool FDConnection::hasData() const
{
    pollfd fd;
    fd.events  = POLLIN;
    fd.fd      = getNotifier();
    EQASSERT( fd.fd > 0 );

    const int nReady = poll( &fd, 1, 0 );
    return nReady > 0;
}

}
#endif
