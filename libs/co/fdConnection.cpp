
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "connectionDescription.h"
#include "exception.h"
#include "global.h"
#include "log.h"

#include <co/base/os.h>

#include <errno.h>
#include <poll.h>

namespace co
{
FDConnection::FDConnection()
        : _readFD( 0 ),
          _writeFD( 0 )
{}

int FDConnection::_getTimeOut()
{
    const uint32_t timeout = Global::getTimeout();
    return timeout == EQ_TIMEOUT_INDEFINITE ? -1 : int( timeout );
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
int64_t FDConnection::readSync( void* buffer, const uint64_t bytes, const bool )
{
    if( _readFD < 1 )
        return -1;

    ssize_t bytesRead = ::read( _readFD, buffer, bytes );
    if( bytesRead > 0 )
        return bytesRead;

    //EQINFO << "1st " << bytesRead << " " << strerror( errno ) << std::endl;
    if( bytesRead == 0 || errno == EWOULDBLOCK || errno == EAGAIN )
    {
        struct pollfd fds[1];
        fds[0].fd = _readFD;
        fds[0].events = POLLIN;

        const int res = poll( fds, 1, _getTimeOut( ));
        if( res < 0 )
        {
            EQWARN << "Error during read : " << strerror( errno ) << std::endl;
            return -1;
        }
        
        if( res == 0 )
            throw Exception( Exception::TIMEOUT_READ );

        bytesRead = ::read( _readFD, buffer, bytes );
        //EQINFO << "2nd " << bytesRead << " " << strerror(errno) << std::endl;
    }

    if( bytesRead > 0 )
        return bytesRead;

    if( bytesRead == 0 ) // EOF
    {
        EQINFO << "Got EOF, closing " << getDescription()->toString()
               << std::endl;
        close();
        return -1;
    }

    EQASSERT( bytesRead == -1 ); // error

    if( errno == EINTR ) // if interrupted, try again
        return 0;

    EQWARN << "Error during read: " << strerror( errno ) << ", " << bytes
           << "b on fd " << _readFD << std::endl;
    return -1;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
int64_t FDConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED || _writeFD < 1 )
        return -1;

    ssize_t bytesWritten = ::write( _writeFD, buffer, bytes );
    if( bytesWritten > 0 )
        return bytesWritten;

    if( bytesWritten == 0 || errno == EWOULDBLOCK || errno == EAGAIN )
    {
        struct pollfd fds[1];
        fds[0].fd = _writeFD;
        fds[0].events = POLLOUT;
        const int res = poll( fds, 1, _getTimeOut( ));
        if (res < 0)
        {
            EQWARN << "Write error : " << strerror( errno ) << std::endl;
            return -1;
        }

        if( res == 0)
            throw Exception( Exception::TIMEOUT_WRITE );

        bytesWritten = ::write( _writeFD, buffer, bytes );
    }

    if( bytesWritten > 0 )
        return bytesWritten;

    if( bytesWritten == -1 ) // error
    {
        if( errno == EINTR ) // if interrupted, try again
            return 0;

        EQWARN << "Error during write: " << strerror( errno ) << std::endl;
        return -1;
    }

    return bytesWritten;
}
}
#endif
