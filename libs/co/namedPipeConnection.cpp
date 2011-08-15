
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <limits>

#include "namedPipeConnection.h"

#include "connectionDescription.h"
#include "exception.h"
#include "global.h"
#include "node.h"

#include <co/base/os.h>
#include <co/base/log.h>

#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/types.h>

#define EQ_PIPE_BUFFER_SIZE 515072
#define EQ_READ_BUFFER_SIZE 257536
#define EQ_WRITE_BUFFER_SIZE 257536

namespace co
{
NamedPipeConnection::NamedPipeConnection()
    : _fd( INVALID_HANDLE_VALUE )
    , _readDone( 0 )
{
    memset( &_read, 0, sizeof( _read ));
    memset( &_write, 0, sizeof( _write ));
    
    _description->type = CONNECTIONTYPE_NAMEDPIPE;
    _description->bandwidth = 768000;
}

NamedPipeConnection::~NamedPipeConnection()
{
    _close();
}

std::string NamedPipeConnection::_getFilename() const
{
    const std::string& filename = _description->getFilename();
    if( filename.find( "\\\\.\\pipe\\" ) == 0 )
        return filename;

    return "\\\\.\\pipe\\" + filename;
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool NamedPipeConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_NAMEDPIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    const std::string filename = _getFilename();
    if ( !WaitNamedPipe( filename.c_str(), 20000 )) 
    {
        EQERROR << "Can't create named pipe: " << base::sysError << std::endl;
        return false;
    }

    if( !_connectNamedPipe( ))
        return false;

    _initAIORead();
    _state = STATE_CONNECTED;
    _fireStateChanged();

    EQINFO << "Connected " << _description->toString() << std::endl;
    return true;
}

void NamedPipeConnection::_close()
{
    if( _state == STATE_CLOSED )
        return;

    EQASSERT( _fd > 0 ); 

    if( isListening( ))
    {
        _exitAIOAccept();

        if( _fd != INVALID_HANDLE_VALUE && !DisconnectNamedPipe( _fd ))
            EQERROR << "Could not disconnect named pipe: " << base::sysError
                    << std::endl;
    }
    else
    {
        _exitAIORead();
        if( _fd != INVALID_HANDLE_VALUE && !CloseHandle( _fd ))
            EQERROR << "Could not close named pipe: " << base::sysError
                    << std::endl;
    }

    _fd = INVALID_HANDLE_VALUE;
    _state = STATE_CLOSED;
    _fireStateChanged();
}

bool NamedPipeConnection::_createNamedPipe()
{
    // Start accept
    const std::string filename = _getFilename();
    _fd = CreateNamedPipe( 
                     filename.c_str(),            // pipe name 
                     PIPE_ACCESS_DUPLEX |         // read/write access 
                     FILE_FLAG_OVERLAPPED,        // overlapped mode 
                     PIPE_TYPE_BYTE |             // message-type  
                     PIPE_READMODE_BYTE |         // message-read  
                     PIPE_WAIT,                   // blocking mode 
                     PIPE_UNLIMITED_INSTANCES,    // number of instances 
                     EQ_PIPE_BUFFER_SIZE,         // output buffer size 
                     EQ_PIPE_BUFFER_SIZE,         // input buffer size 
                     0,                           // default time-out (unused)
                     0 /*&sa*/);                  // default security attributes

    if ( _fd == INVALID_HANDLE_VALUE ) 
    {
        EQERROR << "Could not create named pipe: " 
                << base::sysError << " file : " << filename << std::endl;
        return false;
    }
    return true;
}

bool NamedPipeConnection::_connectNamedPipe()
{
    const std::string filename = _getFilename();
    _fd = CreateFile( 
             filename.c_str(),      // pipe name 
             GENERIC_READ |         // read and write access 
             GENERIC_WRITE, 
             0,                     // no sharing 
             0,                     // default security attributes
             OPEN_EXISTING,         // opens existing pipe 
             FILE_FLAG_OVERLAPPED,  // default attributes 
             0);                    // no template file 

    if( _fd != INVALID_HANDLE_VALUE ) 
       return true;

    if( GetLastError() != ERROR_PIPE_BUSY ) 
    {
        EQERROR << "Can't create named pipe: " << base::sysError << std::endl; 
        return false;
    }

    return _fd != INVALID_HANDLE_VALUE;
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool NamedPipeConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_NAMEDPIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    _initAIOAccept();
    _state = STATE_LISTENING;
    _fireStateChanged();

    EQINFO << "Listening on " << _description->toString() << std::endl;
    return true;
}


bool NamedPipeConnection::_connectToNewClient( HANDLE hPipe ) 
{ 
   // Start an overlapped connection for this pipe instance. 
   const bool fConnected = ConnectNamedPipe( hPipe, &_read ); 
   EQASSERT( !fConnected );
 
   switch( GetLastError() ) 
   { 
      // The overlapped connection in progress. 
      case ERROR_IO_PENDING: 
         return true;
 
      // Client is already connected, so signal an event. 
      case ERROR_PIPE_CONNECTED: 
         if( SetEvent( _read.hEvent ) ) 
            return true; 

      // fall through
      default: 
      {
         EQWARN << "ConnectNamedPipe failed : " << base::sysError << std::endl;
         return false;
      }
   } 
}

//----------------------------------------------------------------------
// Async IO handle
//----------------------------------------------------------------------
void NamedPipeConnection::_initAIORead()
{
    _read.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _read.hEvent );
    _write.hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _write.hEvent );

    if( !_read.hEvent || !_write.hEvent )
        EQERROR << "Can't create events for AIO notification: " 
                << base::sysError  << std::endl;
}

void NamedPipeConnection::_initAIOAccept()
{
    _initAIORead();
}

void NamedPipeConnection::_exitAIOAccept()
{
    _exitAIORead();
}
void NamedPipeConnection::_exitAIORead()
{
    if(  _read.hEvent ) 
    {
        CloseHandle( _read.hEvent );
        _read.hEvent = 0;
    }
    if(  _write.hEvent ) 
    {
        CloseHandle( _write.hEvent );
        _write.hEvent = 0;
    }
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
void NamedPipeConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );
    ResetEvent( _read.hEvent );

    if( _createNamedPipe( ))
        _connectToNewClient( _fd );
    else
        close();
}

ConnectionPtr NamedPipeConnection::acceptSync()
{
    EQ_TS_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
        return 0;

    // complete accept
    DWORD got   = 0;
    if( !GetOverlappedResult( _fd, &_read, &got, TRUE ))
    {
        if (GetLastError() == ERROR_PIPE_CONNECTED) 
        {        
            return 0; 
        }
        EQWARN << "Accept completion failed: " << base::sysError
               << ", closing named pipe" << std::endl;
         
        close();
        return 0;
    }


    base::RefPtr< NamedPipeConnection > newConnection = new NamedPipeConnection;

    newConnection->setDescription( _description );
    newConnection->_fd  = _fd;
    newConnection->_initAIORead();
    newConnection->_state  = STATE_CONNECTED;
    _fd = INVALID_HANDLE_VALUE;

    EQINFO << "accepted connection" << std::endl;
    return newConnection;
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void NamedPipeConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED )
        return;

    ResetEvent( _read.hEvent );
    DWORD use = EQ_MIN( bytes, EQ_READ_BUFFER_SIZE );
    

    if( ReadFile( _fd, buffer, use, &_readDone, &_read ) )
    {
        EQASSERT( _readDone > 0 );
        SetEvent( _read.hEvent );
    }
    else if( GetLastError() != ERROR_IO_PENDING )
    {
        EQWARN << "Could not start overlapped receive: " << base::sysError
               << ", closing connection" << std::endl;
        close();
    }
}

int64_t NamedPipeConnection::readSync( void* buffer, const uint64_t bytes,
                                       const bool ignored )
{
    EQ_TS_THREAD( _recvThread );

    if( _fd == INVALID_HANDLE_VALUE )
    {
        EQERROR << "Invalid read handle" << std::endl;
        return -1;
    }
    
    if( _readDone > 0 )
        return _readDone;

    DWORD got   = 0;
    if( !GetOverlappedResult( _fd, &_read, &got, true ))
    {
        if( GetLastError() == ERROR_PIPE_CONNECTED ) 
            return 0; 

        EQWARN << "Read complete failed: " << base::sysError 
               << ", closing connection" << std::endl;
        close();
        return -1;

    }

    return got;
}

int64_t NamedPipeConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED || _fd == INVALID_HANDLE_VALUE )
        return -1;

    DWORD wrote;
    const DWORD use = EQ_MIN( bytes, EQ_WRITE_BUFFER_SIZE );

    ResetEvent( _write.hEvent );
    if( WriteFile( _fd, buffer, use, &wrote, &_write ))
        return wrote;
    
    if( GetLastError() != ERROR_IO_PENDING )
    {
        EQWARN << "Could not start write: " << base::sysError << std::endl;
        return -1;
    }

    DWORD got = 0;
    if( GetOverlappedResult( _fd, &_write, &got, false ))
        return got;

    switch( GetLastError( ))
    {
      case ERROR_PIPE_CONNECTED:
        return 0;
      case ERROR_IO_PENDING:
      case ERROR_IO_INCOMPLETE:
      {
          const uint32_t timeout = Global::getTimeout();

          if( WAIT_OBJECT_0 != WaitForSingleObject( _write.hEvent, timeout ))
              throw Exception( Exception::TIMEOUT_WRITE );
          break;
      }

      default:
        EQWARN << "Write complete failed: " << base::sysError << std::endl;
    }
        
    if( GetOverlappedResult( _fd, &_write, &got, false ))
        return got;

    if( GetLastError() == ERROR_PIPE_CONNECTED ) 
        return 0;

    EQWARN << "Write complete failed: " << base::sysError << std::endl;
    return -1;
}

}
