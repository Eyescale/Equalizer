
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "pipeConnection.h"

#include "connectionDescription.h"
#include "node.h"

#include <co/base/log.h>
#include <co/base/thread.h>

namespace co
{

PipeConnection::PipeConnection()
#ifdef _WIN32
        : _readHandle( 0 ),
          _writeHandle( 0 ),
          _size( 0 ),
          _dataPending( CreateEvent( 0, TRUE, FALSE, 0 ))
#endif
{
    _description->type = CONNECTIONTYPE_PIPE;
    _description->bandwidth = 1024000;
}

PipeConnection::~PipeConnection()
{
#ifdef _WIN32
    CloseHandle( _dataPending );
    _dataPending = 0;
#endif
}

#ifdef _WIN32

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PipeConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_PIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;
    _size  = 0;

    if( !_createPipe( ))
    {
        close();
        return false;
    }

    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;
}

bool PipeConnection::_createPipe()
{
    if( CreatePipe( &_readHandle, &_writeHandle, 0, 0 ) == 0 )
    {
        EQERROR << "Could not create pipe: " << co::base::sysError 
                << std::endl;
        close();
        return false;
    }
    return true;
}

void PipeConnection::close()
{
    if( _writeHandle )
    {
        CloseHandle( _writeHandle );
        _writeHandle = 0;
    }
    if( _readHandle )
    {
        CloseHandle( _readHandle );
        _readHandle = 0;
    }
    _state = STATE_CLOSED;
    _fireStateChanged();
}
void PipeConnection::readNB( void* buffer, const uint64_t bytes ) { /* NOP */ }
int64_t PipeConnection::readSync( void* buffer, const uint64_t bytes,
                                  const bool ignored )
{
    if( !_readHandle )
        return -1;

    DWORD bytesRead = 0;
    const BOOL ret = ReadFile( _readHandle, buffer, static_cast<DWORD>( bytes ),
                               &bytesRead, 0 );

    if( ret == 0 ) // Error
    {
        EQWARN << "Error during read: " << co::base::sysError << std::endl;
        return -1;
    }

    if( bytesRead == 0 ) // EOF
    {
        close();
        return -1;
    }

    _mutex.set();
    EQASSERT( _size >= bytesRead );
    _size -= bytesRead;
    if( _size == 0 )
        ResetEvent( _dataPending );
    _mutex.unset();

    return bytesRead;
}

int64_t PipeConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED || !_writeHandle )
        return -1;

    const DWORD size = EQ_MIN( static_cast<DWORD>( bytes ), 4096 );

    _mutex.set();
    _size += size; // speculatively 'write' everything
    _mutex.unset();

    DWORD bytesWritten = 0;
    const BOOL ret = WriteFile( _writeHandle, buffer, size, &bytesWritten, 0 );

    if( ret == 0 ) // Error
    {
        EQWARN << "Error during write: " << co::base::sysError << std::endl;
        bytesWritten = 0;
    }

    _mutex.set();
    EQASSERT( _size >= size - bytesWritten );
    _size -= ( size - bytesWritten ); // correct size

    if( _size > 0 )
        SetEvent( _dataPending );
    else
    {
        EQASSERT( _size == 0 );
        ResetEvent( _dataPending );
    }
    _mutex.unset();

    if( ret==0 ) 
        return -1;
    return bytesWritten;
}

#else // !_WIN32

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PipeConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_PIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createPipe( ))
    {
        close();
        return false;
    }

    EQVERB << "readFD " << _readFD << " writeFD " << _writeFD << std::endl;
    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;
}

bool PipeConnection::_createPipe()
{
    int pipeFDs[2];
    if( ::pipe( pipeFDs ) == -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }

    _readFD  = pipeFDs[0];
    _writeFD = pipeFDs[1];
    return true;
}

void PipeConnection::close()
{
    if( _writeFD > 0 )
    {
        ::close(_writeFD);
        _writeFD = 0;
    }
    if( _readFD > 0 )
    {
        ::close(_readFD);
        _readFD  = 0;
    }

    _state = STATE_CLOSED;
    _fireStateChanged();
}

#endif
}
