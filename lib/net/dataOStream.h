
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQNET_DATAOSTREAM_H
#define EQNET_DATAOSTREAM_H

#include <eq/base/buffer.h> // member
#include <eq/net/types.h>

#include <iostream>
#include <vector>

namespace eq
{
namespace net
{
namespace DataStreamTest
{
    class Sender;
}
    class Connection;

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
        EQ_EXPORT DataOStream();
        virtual EQ_EXPORT ~DataOStream();

        /** Enable output, locks the connections to the receivers */ 
        void enable( const Nodes& receivers );
        void enable( NodePtr node, const bool useMulticast );
        EQ_EXPORT void enable();

        /** Resend the saved buffer. */
        void resend( NodePtr node );

        /** Disable, flush and unlock the output to the current receivers. */
        EQ_EXPORT void disable();

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
        /** Write a plain data item by copying it to the stream. */
        template< typename T > DataOStream& operator << ( const T& value )
            { write( &value, sizeof( value )); return *this; }

        /** Write a std::vector of serializable items. */
        template< typename T >
        DataOStream& operator << ( const std::vector< T >& value );

        /** Write a number of bytes from data into the stream. */
        EQ_EXPORT void write( const void* data, uint64_t size );

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
        /** Flush remaining data in the buffer. */
        void _flush();

        /** @name Packet sending, implemented by the subclasses */
        //@{
        /** Send a data buffer (packet) to the receivers. */
        virtual void sendData( const uint32_t compressor,
                               const uint32_t nChunks,
                               const void* const* chunks,
                               const uint64_t* chunkSizes,
                               const uint64_t sizeUncompressed ) = 0;
                                 
        /** Send the trailing data (packet) to the receivers */
        virtual void sendFooter( const uint32_t compressor,
                                 const uint32_t nChunks,
                                 const void* const* chunks, 
                                 const uint64_t* chunkSizes,
                                 const uint64_t sizeUncompressed ) = 0;
        //@}

        /** Reset the whole stream. */
        virtual EQ_EXPORT void reset();

        /** Locked connections to the receivers, if _enabled */
        Connections _connections;
        friend class DataStreamTest::Sender;

        base::CPUCompressor* const compressor;

    private:        
        enum BufferType
        {
            BUFFER_NONE = 0,
            BUFFER_PARTIAL,
            BUFFER_ALL
        };
        BufferType _bufferType;
        
        /** The buffer used for saving and buffering */
        base::Bufferb  _buffer;
        /** The start position of the buffering, always 0 if !_save */
        uint64_t _bufferStart;
        
        /** The output stream is enabled for writing */
        bool _enabled;
        /** Some data has been sent since it was _enabled */
        bool _dataSent;

        /** Save all sent data */
        bool _save;

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
        uint64_t _getCompressedData( void** chunks, uint64_t* chunkSizes )const;

        /** compress data, if compressor found */
        void _compress( const void* src, const uint64_t size );

    };

    std::ostream& operator << ( std::ostream& os,
                                const DataOStream& dataOStream );

}
}

#include <eq/net/object.h>
#include <eq/net/objectVersion.h>

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
            EQASSERTINFO( !child || child->getID() <= EQ_ID_MAX,
                          "Found unmapped object during serialization" );
        }
    }
/** @endcond */

    /** Optimized specialization to write a std::vector of uint8_t. */
    template<> inline DataOStream& 
    DataOStream::operator << ( const std::vector< uint8_t >& value )
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
}
#endif //EQNET_DATAOSTREAM_H
