
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DATAOSTREAM_H
#define EQNET_DATAOSTREAM_H

#include <eq/base/buffer.h> // member
#include <eq/net/types.h>   // ConnectionVector member

#include <iostream>
#include <vector>

namespace eq
{
namespace net
{
    class Connection;

    /**
     * A std::ostream buffering and/or retaining data in a binary format.
     *
     * Derived classes send the data using command packets.
     */
    class EQ_EXPORT DataOStream
    {
    public:
        DataOStream();
        virtual ~DataOStream();

        /** Enable output, locks the connections to the receivers */ 
        void enable( const NodeVector& receivers );
        void enable( const ConnectionVector& receivers );
        void enable( const NodePtr node );
        void enable();

        /** Resend the saved buffer. */
        void resend( const NodePtr node );

        /** Disable, flush and unlock the output to the current receivers. */
        void disable();

        /** Flush the buffer. */
        void flush();

        /** Enable aggregation/copy of data before sending it. */
        void enableBuffering();
        /** Disable aggregation/copy of data before sending it. */
        void disableBuffering();
        
        /** Enable copying of all data into a saved buffer. */
        void enableSave();
        /** Disable copying of all data into a saved buffer. */
        void disableSave();

        /** @name Data Access. */
        //*{
        /** @return if data was sent since the last enable() */
        bool hasSentData() const { return _dataSent; }

        /** @return the buffer with the saved data. */
        const base::Bufferb& getSaveBuffer() const 
            { EQASSERT( _save ); return _buffer; }
        //*}

        /** @name Basic data output */
        //*{
        template< typename T >
        DataOStream& operator << ( const T& value )
            { write( &value, sizeof( value )); return *this; }

        template< typename T >
        DataOStream& operator << ( const std::vector< T >& value )
            { 
                const uint64_t nElems = value.size();
                write( &nElems, sizeof( nElems ));
                if( nElems > 0 )
                    write( &value[0], nElems * sizeof( T ) );
                return *this;
            }

        void write( const void* data, uint64_t size );
        void writeOnce( const void* data, uint64_t size );
        //*}

 
    protected:
        /** Send the leading data (packet) to the receivers */
        virtual void sendHeader( const void* buffer, const uint64_t size ) = 0;
        /** Send a data buffer (packet) to the receivers. */
        virtual void sendBuffer( const void* buffer, const uint64_t size ) = 0;
        /** Send the trailing data (packet) to the receivers */
        virtual void sendFooter( const void* buffer, const uint64_t size ) = 0;
        /** Send only one data item (packet) to the receivers */
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { sendHeader( buffer, size ); sendFooter( 0, 0 ); }

        /** Locked connections to the receivers, if _enabled */
        ConnectionVector _connections;

    private:
        /** The buffer used for saving and buffering */
        base::Bufferb  _buffer;
        /** The start position of the buffering, always 0 if !_save */
        uint64_t _bufferStart;
        /** The threshold for the buffer to flush */
        static uint64_t _highWaterMark;
        
        /** The output stream is enabled for writing */
        bool _enabled;
        /** Some data has been sent since it was _enabled */
        bool _dataSent;
        /** Use send buffering */
        bool _buffered;
        /** Save all sent data */
        bool _save;

        /** Helper function calling sendHeader and sendBuffer as needed. */
        void _sendBuffer( const void* data, const uint64_t size );
        
        /** Reset the start position after sending a buffer. */
        void _resetStart();

        /** Unlock all connections during disable. */
        void _unlockConnections();
    };

    // Some template specializations
    template<>
    inline DataOStream& DataOStream::operator << ( const std::string& str )
    { 
        const uint64_t nElems = str.length();
        write( &nElems, sizeof( nElems ));
        if ( nElems > 0 )
            write( str.c_str(), nElems );

        return *this;
    }
}
}
#endif //EQNET_DATAOSTREAM_H
