
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DATAOSTREAM_H
#define EQNET_DATAOSTREAM_H

#include <eq/base/buffer.h> // member
#include <eq/net/types.h>   // ConnectionVector member

#include <iostream>
#include <vector>

namespace eqNet
{
    class Connection;

    /**
     * A std::ostream buffering and/or retaining data in a binary format.
     *
     * Derived classes send the data using command packets.
     */
    class EQ_EXPORT DataOStream // : public std::ostream
    {
    public:
        DataOStream();
        virtual ~DataOStream();

        /** Enable output, locks the connections to the receivers */ 
        void enable( const NodeVector& receivers );
        void enable( const ConnectionVector& receivers );

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

        /** Swap the saved data with the buffer. */
        void swapSaveBuffer( eqBase::Buffer& buffer );


        /** @name Basic data output */
        //*{
        DataOStream& operator << ( const int value )
            { write( &value, sizeof( value )); return *this; }

        template< typename T >
        DataOStream& operator << ( const std::vector< T >& value )
            { 
                const uint64_t nElems = value.size();
                write( &nElems, sizeof( nElems ));
                write( &value[0], nElems * sizeof( T ) );
                return *this;
            }

        void write( const void* data, uint64_t size );
        //*}

 
    protected:
        /** Send the leading data (packet) to the receivers */
        virtual void sendHeader() = 0;
        /** Send a data buffer (packet) to the receivers. */
        virtual void sendBuffer( const void* buffer, const uint64_t size ) = 0;
        /** Send the trailing data (packet) to the receivers */
        virtual void sendFooter() = 0;

        /** Locked connections to the receivers, if _enabled */
        ConnectionVector _connections;

    private:
        /** The buffer used for saving and buffering */
        eqBase::Buffer  _buffer;
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
    };
}
#endif //EQNET_DATAOSTREAM_H
