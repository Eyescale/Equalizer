
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fdConnection.h"
#include "log.h"

#include <eq/base/scopedMutex.h>

#include <errno.h>

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
uint64_t FDConnection::recv( void* buffer, const uint64_t bytes )
{
    EQLOG( LOG_WIRE ) << "Receiving " << bytes << " bytes on " << this << endl;
    if( _state != STATE_CONNECTED || _readFD == -1 )
        return 0;

    unsigned char* ptr       = static_cast<unsigned char*>(buffer);
    uint64_t       bytesLeft = bytes;

    while( bytesLeft )
    {
        EQASSERT( _readFD > 0 );
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

    if( eqBase::Log::topics & LOG_WIRE ) // OPT
    {
        EQLOG( LOG_WIRE ) << disableFlush << "Received " << bytes << " bytes: ";
        const uint32_t printBytes = MIN( bytes, 256 );
        unsigned char* data       = static_cast<unsigned char*>(buffer);

        for( uint32_t i=0; i<printBytes; i++ )
        {
            if( i%4 )
                EQLOG( LOG_WIRE ) << " ";
            else if( i )
                EQLOG( LOG_WIRE ) << "|";

            EQLOG( LOG_WIRE ) << static_cast<int>(data[i]);
        }
        if( printBytes < bytes ) 
            EQLOG( LOG_WIRE ) << "|...";
        EQLOG( LOG_WIRE ) << endl << enableFlush;
    }
            
    return bytes;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
uint64_t FDConnection::send( const void* buffer, const uint64_t bytes, 
                             bool isLocked ) const
{
    if( _state != STATE_CONNECTED || _writeFD == -1 )
        return 0;


    // possible OPT: We need to lock here to guarantee an atomic transmission of
    // the buffer. Two possible improvements are:
    // 1) Use a spinlock, since this lock should be relatively uncontended
    // 2) Disassemble buffer into 'small enough' pieces and use a header to
    //    reassemble correctly on the other side (aka reliable UDP)
    // 3) Introduce a send thread with a thread-safe task queue
    ScopedMutex mutex( isLocked ? 0 : &_sendLock );

    const unsigned char* ptr       = static_cast<const unsigned char*>(buffer);
    uint64_t       bytesLeft = bytes;

    if( eqBase::Log::topics & LOG_WIRE ) // OPT
    {
        EQLOG( LOG_WIRE ) << disableFlush << "Sending " << bytes 
                          << " bytes on " << (void*)this << ":";
        const uint32_t printBytes = MIN( bytes, 256 );

        for( uint32_t i=0; i<printBytes; i++ )
        {
            if( i%4 )
                EQLOG( LOG_WIRE ) << " ";
            else if( i )
                EQLOG( LOG_WIRE ) << "|";

            EQLOG( LOG_WIRE ) << static_cast<int>(ptr[i]);
        }
        if( printBytes < bytes ) 
            EQLOG( LOG_WIRE ) << "|...";
        EQLOG( LOG_WIRE ) << endl << enableFlush;
    }

    while( bytesLeft )
    {
        EQASSERT( _writeFD > 0 );
        const ssize_t bytesWritten = ::write( _writeFD, ptr, bytesLeft );

        if( bytesWritten == -1 ) // error
        {
            if( errno != EINTR ) // if interrupted, try again
            {
                EQWARN << "Error during write: " << strerror( errno ) << endl;
                return bytes - bytesLeft;
            }
        }
        else
        {
            bytesLeft -= bytesWritten;
            ptr += bytesWritten;
        }
    }

    return bytes;
}
