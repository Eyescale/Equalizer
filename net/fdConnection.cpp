
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fdConnection.h"

#include <sys/errno.h>

using namespace eqNet;
using namespace std;

FDConnection::FDConnection()
        : _readFD( -1 ),
          _writeFD( -1 )
{
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
size_t FDConnection::read( const void* buffer, const size_t bytes )
{
    if( _state != STATE_CONNECTED || _readFD == -1 )
        return 0;

    char* ptr = (char*)buffer;
    size_t bytesLeft = bytes;

    while( bytesLeft )
    {
        ssize_t bytesRead = ::read( _readFD, ptr, bytesLeft );
        
        if( bytesRead == 0 ) // EOF
        {
            close();
            return bytes - bytesLeft;
        }

        if( bytesRead == -1 ) // error
        {
            if( errno == EINTR ) // if interrupted, try again
                bytesRead = 0;
            else
            {
                WARN( "Error during socket read: %s\n", strerror( errno ));
                return bytes - bytesLeft;
            }
        }
        
        bytesLeft -= bytesRead;
        ptr += bytesRead;
    }

    return bytes;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
size_t FDConnection::write( const void* buffer, const size_t bytes )
{
    if( _state != STATE_CONNECTED || _writeFD == -1 )
        return 0;

    char* ptr = (char*)buffer;
    size_t bytesLeft = bytes;

    while( bytesLeft )
    {
        ssize_t bytesWritten = ::write( _writeFD, ptr, bytesLeft );
        
        if( bytesWritten == -1 ) // error
        {
            if( errno == EINTR ) // if interrupted, try again
                bytesWritten = 0;
            else
            {
                WARN( "Error during socket write: %s\n", strerror( errno ));
                return bytes - bytesLeft;
            }
        }
        
        bytesLeft -= bytesWritten;
        ptr += bytesWritten;
    }

    return bytes;
}
