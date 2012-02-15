
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef CO_DATAOSTREAM_H
#define CO_DATAOSTREAM_H

#include <co/api.h>
#include <co/types.h>
#include <co/base/buffer.h> // member
#include <co/base/nonCopyable.h> // base class

#include <iostream>
#include <vector>

//#define EQ_INSTRUMENT_DATAOSTREAM

namespace co
{
namespace DataStreamTest
{
    class Sender;
}

    /**
     * A std::ostream buffering and/or retaining data in a binary format.
     *
     * Derived classes send the data using the appropriate command packets.
     */
    class DataOStream : public base::NonCopyable
    {
    public:
        /** @name Internal */
        //@{
        CO_API DataOStream();
        virtual CO_API ~DataOStream();

        /** Disable and flush the output to the current receivers. */
        CO_API void disable();

        /** Disable, then send the packet to the current receivers. */
        void disable( const Packet& packet );

        /** Enable copying of all data into a saved buffer. */
        void enableSave();

        /** Disable copying of all data into a saved buffer. */
        void disableSave();

        /** @return if data was sent since the last enable() */
        bool hasSentData() const { return _dataSent; }
        //@}

        /** @name Data output */
        //@{
        /** Write a plain data item by copying it to the stream. */
        template< typename T > DataOStream& operator << ( const T& value )
            { write( &value, sizeof( value )); return *this; }

        /** Write a std::vector of serializable items. */
        template< typename T >
        DataOStream& operator << ( const std::vector< T >& value );

        /** Write a number of bytes from data into the stream. */
        CO_API void write( const void* data, uint64_t size );

        /**
         * Serialize child objects.
         *
         * The DataIStream has a deserialize counterpart to this method. All
         * child objects have to be registered or mapped beforehand.
         */
        template< typename C >
        void serializeChildren( const std::vector< C* >& children );
        //@}
 
    protected:
        /** Initialize the given compressor. */
        void _initCompressor( const uint32_t compressor );

        /** Enable output. */
        CO_API void _enable();

        /** Flush remaining data in the buffer. */
        void _flush();

        /**
         * Set up the connection list for a group of  nodes, using multicast
         * where possible.
         */
        void _setupConnections( const Nodes& receivers );

        /** Set up the connection (list) for one node. */
        void _setupConnection( NodePtr node, const bool useMulticast );

        /** Resend the saved buffer to all enabled connections. */
        void _resend();

        void _send( const Packet& packet );

        void _clearConnections() { _connections.clear(); }

        /** @name Packet sending, used by the subclasses */
        //@{
        /** Send a data buffer (packet) to the receivers. */
        virtual void sendData( const void* buffer, const uint64_t size,
                               const bool last ) = 0;

        template< typename P >
        void sendPacket( P& packet, const void* buffer, const uint64_t size,
                         const bool last );
        //@}

        /** Reset the whole stream. */
        virtual CO_API void reset();

    private:        
        enum CompressorState
        {
            STATE_UNCOMPRESSED,
            STATE_PARTIAL,
            STATE_COMPLETE,
            STATE_UNCOMPRESSIBLE,
        };
        CompressorState _compressorState;
        
        /** The buffer used for saving and buffering */
        base::Bufferb  _buffer;

        /** The start position of the buffering, always 0 if !_save */
        uint64_t _bufferStart;

        /** The uncompressed size of a completely compressed buffer. */
        uint64_t _dataSize;

        /** Locked connections to the receivers, if _enabled */
        Connections _connections;
        friend class DataStreamTest::Sender;

        /** The compressor instance. */
        base::CPUCompressor* const _compressor;

        /** The output stream is enabled for writing */
        bool _enabled;

        /** Some data has been sent since it was _enabled */
        bool _dataSent;

        /** Save all sent data */
        bool _save;

        bool _disable();

        /** Helper function preparing data for sendData() as needed. */
        void _sendData( const void* data, const uint64_t size );
        
        /** Reset after sending a buffer. */
        void _resetBuffer();

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
        /** Send the trailing data (packet) to the receivers */
        void _sendFooter( const void* buffer, const uint64_t size );

        /**
         * Collect compressed data.
         * @return the total size of the compressed data.
         */
        CO_API uint64_t _getCompressedData( void** chunks,
                                            uint64_t* chunkSizes ) const;

        /** Compress data and update the compressor state. */
        void _compress( void* src, const uint64_t size,
                        const CompressorState result );
    };

    std::ostream& operator << ( std::ostream& os,
                                const DataOStream& dataOStream );

}

#include <co/object.h>
#include <co/objectVersion.h>

namespace co
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

    /** Write an object identifier and version. */
    template<> inline DataOStream& 
    DataOStream::operator << ( const Object* const& object )
    {
        EQASSERT( !object || object->isAttached( ));
        (*this) << ObjectVersion( object );
        return *this;
    }
 
/** @cond IGNORE */
    /** Write a std::vector of serializable items. */
    template< typename T > inline DataOStream& 
    DataOStream::operator << ( const std::vector< T >& value )
    {
        const uint64_t nElems = value.size();
        (*this) << nElems;
        for( uint64_t i = 0; i < nElems; ++i )
            (*this) << value[i];
        return *this;
    }
 
    template< typename C > inline void
    DataOStream::serializeChildren( const std::vector<C*>& children )
    {
        const uint64_t nElems = children.size();
        (*this) << nElems;

        for( typename std::vector< C* >::const_iterator i = children.begin();
             i != children.end(); ++i )
        {
            C* child = *i;
            (*this) << ObjectVersion( child );
            EQASSERTINFO( !child || child->isAttached(),
                          "Found unmapped object during serialization" );
        }
    }
/** @endcond */

    /** Optimized specialization to write a std::vector of uint8_t. */
    template<> inline DataOStream& 
    DataOStream::operator << ( const std::vector< uint8_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of uint16_t. */
    template<> inline DataOStream& 
    DataOStream::operator << ( const std::vector< uint16_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of int16_t. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< int16_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of uint32_t. */
    template<> inline DataOStream& 
    DataOStream::operator << ( const std::vector< uint32_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of int32_t. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< int32_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of uint64_t. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< uint64_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of int64_t. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< int64_t >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of float. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< float >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of double. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< double >& value )
    { return _writeFlatVector( value ); }

    /** Optimized specialization to write a std::vector of ObjectVersion. */
    template<> inline DataOStream&
    DataOStream::operator << ( const std::vector< ObjectVersion >& value )
    { return _writeFlatVector( value ); }
    //@}
}
#endif //CO_DATAOSTREAM_H
