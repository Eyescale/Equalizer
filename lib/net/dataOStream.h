
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
     * Derived classes send the data using the appropriate command packets.
     */
    class DataOStream
    {
    public:
        /** @name Internal */
        //@{
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

        /** Enable aggregation/copy of data before sending it. */
        void enableBuffering();
        /** Disable aggregation/copy of data before sending it. */
        void disableBuffering();
        
        /** Enable copying of all data into a saved buffer. */
        void enableSave();
        /** Disable copying of all data into a saved buffer. */
        void disableSave();

        /** @return if data was sent since the last enable() */
        bool hasSentData() const { return _dataSent; }

        /** @return the buffer with the saved data. */
        const base::Bufferb& getSaveBuffer() const 
            { EQASSERT( _save ); return _buffer; }
        //@}

        /** @name Data output */
        //@{
        /** Flush remaining data in the buffer. */
        void flush();

        /** Write a plain data item by copying it to the stream. */
        template< typename T >
        DataOStream& operator << ( const T& value )
            { write( &value, sizeof( value )); return *this; }

        /** Write a std::vector of serializable items. */
        template< typename T >
        DataOStream& operator << ( const std::vector< T >& value )
            {
                const uint64_t nElems = value.size();
                write( &nElems, sizeof( nElems ));
                for( uint64_t i =0; i < nElems; ++i )
                    (*this) << value[i];
                return *this;
            }

        /** Write a number of bytes from data into the stream. */
        EQ_EXPORT void write( const void* data, uint64_t size );

        /** Write one block of data into the stream and close it immediately. */
        EQ_EXPORT void writeOnce( const void* data, uint64_t size );
        //@}

 
    protected:
        /** @name Packet sending, implemented by the subclasses */
        //@{
        /** Send the leading data (packet) to the receivers */
        virtual void sendHeader( const void* buffer, const uint64_t size ) = 0;
        /** Send a data buffer (packet) to the receivers. */
        virtual void sendBuffer( const void* buffer, const uint64_t size ) = 0;
        /** Send the trailing data (packet) to the receivers */
        virtual void sendFooter( const void* buffer, const uint64_t size ) = 0;
        /** Send only one data item (packet) to the receivers */
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { sendHeader( buffer, size ); sendFooter( 0, 0 ); }
        //@}

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

        /** Write a vector of trivial data. */
        template< typename T > 
        DataOStream& _writeFlatVector( const std::vector< T >& value )
        {
            const uint64_t nElems = value.size();
            write( &nElems, sizeof( nElems ));
            if( nElems > 0 )
                write( &value.front(), nElems * sizeof( T ));
            return *this;
        }
    };

}
}

namespace eq
{
namespace net
{
    /** @name Specialized output operators */
    //@{
    /** Write a std::string. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::string& str )
    { 
        const uint64_t nElems = str.length();
        write( &nElems, sizeof( nElems ));
        if ( nElems > 0 )
            write( str.c_str(), nElems );

        return *this;
    }

    /** Write a base::UUID. */
    template<>
    inline DataOStream& DataOStream::operator << ( const base::UUID& id )
    { 
        base::UUID out( id );
        out.convertToNetwork();
        write( &out, sizeof( out ));
        return *this;
    }

    /** Optimized specialization to write a std::vector of uint8_t. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< uint8_t >&
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of uint32_t. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< uint32_t>&
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of int32_t. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< int32_t >&
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of uint64_t. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< uint64_t>&
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of int64_t. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< int64_t >&
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of float. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< float >& 
                                                   value )
    {
        return _writeFlatVector( value );
    }

    /** Optimized specialization to write a std::vector of double. */
    template<>
    inline DataOStream& DataOStream::operator << ( const std::vector< double >& 
                                                   value )
    {
        return _writeFlatVector( value );
    }
    //@}
}
}
#endif //EQNET_DATAOSTREAM_H
