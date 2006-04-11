
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fdConnection.h"

#include <eq/base/log.h>

#include <errno.h>
#include <unistd.h>

using namespace eqBase;
using namespace eqNet;
using namespace std;

FDConnection::FDConnection()
        : _readFD( -1 ),
          _writeFD( -1 )
{}

FDConnection::FDConnection( const FDConnection& conn )
        : Connection( conn ),
          _readFD( conn._readFD ),
          _writeFD( conn._writeFD )
{}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
uint64_t FDConnection::recv( const void* buffer, const uint64_t bytes )
{
    EQVERB << "Receiving " << bytes << " bytes on " << this << endl;
    if( _state != STATE_CONNECTED || _readFD == -1 )
        return 0;

    char*    ptr       = (char*)buffer;
    uint64_t bytesLeft = bytes;

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
                EQWARN << "Error during read: " << strerror( errno ) << endl;
                return bytes - bytesLeft;
            }
        }
        
        bytesLeft -= bytesRead;
        ptr += bytesRead;
    }

    if( eqBase::Log::level >= eqBase::LOG_VERBATIM ) // OPT
    {
        EQVERB << disableFlush << "Received " << bytes << " bytes: ";
        const char* data = (char*)buffer;

        for(uint64_t i=0; i<bytes; i++ )
        {
            if( i%4 )
                EQVERB << " ";
            else if( i )
                EQVERB << "|";

            EQVERB << (int)data[i];
        }
        EQVERB << endl << enableFlush;
    }
            
    return bytes;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
uint64_t FDConnection::send( const void* buffer, const uint64_t bytes ) const
{
    if( _state != STATE_CONNECTED || _writeFD == -1 )
        return 0;

    char*    ptr       = (char*)buffer;
    uint64_t bytesLeft = bytes;

    if( eqBase::Log::level >= eqBase::LOG_VERBATIM ) // OPT
    {
        EQVERB << disableFlush
               << "Sending " << bytes << " bytes on " << (void*)this << ":";

        for( uint64_t i=0; i<bytes; i++ )
        {
            if( i%4 )
                EQVERB << " ";
            else if( i )
                EQVERB << "|";

            EQVERB << (int)ptr[i];
        }
        EQVERB << endl << enableFlush;
    }

    while( bytesLeft )
    {
        ssize_t bytesWritten = ::write( _writeFD, ptr, bytesLeft );
        
        if( bytesWritten == -1 ) // error
        {
            if( errno == EINTR ) // if interrupted, try again
                bytesWritten = 0;
            else
            {
                EQWARN << "Error during write: " << strerror( errno ) << endl;
                return bytes - bytesLeft;
            }
        }
        
        bytesLeft -= bytesWritten;
        ptr += bytesWritten;
    }

    return bytes;
}
