
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

#ifndef EQNET_DATAISTREAM_H
#define EQNET_DATAISTREAM_H

#include <eq/net/types.h>
#include <eq/base/buffer.h> // member
#include "dataStream.h"     // Base Class
#include <iostream>
#include <vector>

namespace eq
{
namespace net
{
    
    /** A std::istream-like input data stream for binary data. */
    class DataIStream : public DataStream
    {
    public:
        /** @name Internal */
        //@{ 
        DataIStream();
        DataIStream( const DataIStream& from );
        virtual ~DataIStream();

        /** Get the number of remaining buffers. */
        virtual size_t nRemainingBuffers() const = 0;

        virtual uint32_t getVersion() const = 0;

        virtual void reset();
        //@}

        /** @name Data input */
        //@{
        /** Read a plain data item. */
        template< typename T >
        DataIStream& operator >> ( T& value )
            { read( &value, sizeof( value )); return *this; }

        /** Read a std::vector of serializable items. */
        template< typename T >
        DataIStream& operator >> ( std::vector< T >& value )
        {
            uint64_t nElems = 0;
            read( &nElems, sizeof( nElems ));
            value.resize( nElems );
            for( uint64_t i = 0; i < nElems; i++ )
                (*this) >> value[i];

            return *this; 
        }

        /** Read a number of bytes from the stream into a buffer.  */
        EQ_EXPORT void read( void* data, uint64_t size );

        /** 
         * Get the pointer to the remaining data in the current buffer.
         *
         * The data written by the DataOStream on the other end is bucketized,
         * that is, it is sent in multiple blocks. The remaining buffer and its
         * size points into one of the buffers, that is, not all the data sent
         * is returned by this function. However, a write operation on the other
         * end is never segmented, that is, if the application writes n bytes to
         * the DataOStream, a symmetric read from the DataIStream has at least n
         * bytes available.
         */
        EQ_EXPORT const void*    getRemainingBuffer();

        /** Get the size of the remaining data in the current buffer. */
        EQ_EXPORT uint64_t       getRemainingBufferSize();

        /** Advance the current buffer by a number of bytes. */
        EQ_EXPORT void           advanceBuffer( const uint64_t offset ); 
        //@}
 
    protected:
        virtual bool getNextBuffer( const uint8_t** buffer, uint64_t* size ) =0;

        void _decompress( void* src, 
                          const uint8_t** dst, 
                          const uint32_t name,
                          const uint32_t nChunks,
                          const uint64_t dataSize );

    private:
        /** The current input buffer */
        const uint8_t* _input;
        /** The size of the input buffer */
        uint64_t _inputSize;
        /** The current read position in the buffer */
        uint64_t  _position;

        void* _decompressor;   //!< the instance of the decompressor
        eq::base::Bufferb _datas; //!< a buffer for decompress datas

        void _initDecompressor( const uint32_t name );
        /**
         * Check that the current buffer has data left, get the next buffer is
         * necessary, return false if no data is left. 
         */
        bool _checkBuffer();

        /** Read a vector of trivial data. */
        template< typename T >
        DataIStream& _readFlatVector ( std::vector< T >& value )
        {
            uint64_t nElems = 0;
            read( &nElems, sizeof( nElems ));
            value.resize( nElems );
            if( nElems > 0 )
                read( &value.front(), nElems * sizeof( T ));            
            return *this; 
        }
    };
}
}

namespace eq
{
namespace net
{
    /** @name Specialized input operators */
    //@{
    /** Read a std::string. */
    template<>
    inline DataIStream& DataIStream::operator >> ( std::string& str )
    { 
        uint64_t nElems = 0;
        read( &nElems, sizeof( nElems ));
        EQASSERT( nElems <= getRemainingBufferSize( ));
        if( nElems == 0 )
            str.clear();
        else
        {
            str.assign( static_cast< const char* >( getRemainingBuffer( )), 
                        nElems );
            advanceBuffer( nElems );
        }
        return *this; 
    }

    /** Optimized specialization to read a std::vector of uint8_t. */
    template<>
    inline DataIStream& DataIStream::operator >> (std::vector< uint8_t >& value)
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of uint32_t. */
    template<>
    inline DataIStream& DataIStream::operator >> (std::vector< uint32_t>& value)
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of int32_t. */
    template<>
    inline DataIStream& DataIStream::operator >> (std::vector< int32_t >& value)
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of uint64_t. */
    template<>
    inline DataIStream& DataIStream::operator >> (std::vector< uint64_t>& value)
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of int64_t. */
    template<>
    inline DataIStream& DataIStream::operator >> (std::vector< int64_t >& value)
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of float. */
    template<>
    inline DataIStream& DataIStream::operator >> ( std::vector< float >& value )
    {
        return _readFlatVector( value );
    }

    /** Optimized specialization to read a std::vector of double. */
    template<>
    inline DataIStream& DataIStream::operator >> ( std::vector< double >& value)
    {
        return _readFlatVector( value );
    }
    //@}
}
}

#endif //EQNET_DATAISTREAM_H
