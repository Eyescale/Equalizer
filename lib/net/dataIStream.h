
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DATAISTREAM_H
#define EQNET_DATAISTREAM_H

#include <eq/net/object.h>  // nested VERSION_NONE enum
#include <eq/base/buffer.h> // member

#include <iostream>
#include <vector>

namespace eq
{
namespace net
{
    /**
     * A std::istream-like input data stream for binary data.
     *
     * Derived classes send the data using command packets.
     */
    class EQ_EXPORT DataIStream
    {
    public:
        DataIStream();
        virtual ~DataIStream();

        /** @name Basic data input */
        //*{ 
        /** Read a POD data item. */
        template< typename T >
        DataIStream& operator >> ( T& value )
            { read( &value, sizeof( value )); return *this; }

        /** Read a std::vector of POD data items. */
        template< typename T >
        DataIStream& operator >> ( std::vector< T >& value )
        {
            uint64_t nElems = 0;
            read( &nElems, sizeof( nElems ));
            value.resize( nElems );
            if( nElems > 0 )
                read( &value[0], nElems * sizeof( T ) ); 
            return *this; 
        }

        /** Read a number of bytes into a buffer.  */
        void read( void* data, uint64_t size );

        /** Get the pointer to the data remaining in the current buffer. */
        const void*    getRemainingBuffer();

        /** Get the size of the data remaining in the current buffer. */
        uint64_t       getRemainingBufferSize();

        /** Advance the current buffer by a number of bytes. */
        void           advanceBuffer( const uint64_t offset ); 

        /** Get the number of remaining buffers. */
        virtual size_t nRemainingBuffers() const = 0;

        virtual uint32_t getVersion() const { return Object::VERSION_NONE; }
        //*}

        virtual void reset();
 
    protected:
        virtual bool getNextBuffer( const uint8_t** buffer, uint64_t* size ) =0;

    private:
        /** The current input buffer */
        const uint8_t* _input;
        /** The size of the input buffer */
        uint64_t _inputSize;
        /** The current read position in the buffer */
        uint64_t  _position;

        /**
         * Check that the current buffer has data left, get the next buffer is
         * necessary, return false if no data is left. 
         */
        bool _checkBuffer();
    };
}
}

#include <eq/net/nodeID.h>
namespace eq
{
namespace net
{
    // Some template specializations
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
            str.assign( static_cast< const char* >(getRemainingBuffer( )), nElems );
            advanceBuffer( nElems );
        }
        return *this; 
    }
    template<>
    inline DataIStream& DataIStream::operator >> ( NodeID& nodeID )
    { 
        read( &nodeID, sizeof( nodeID ));
        nodeID.convertToHost();
        return *this;
    }
}
}

#endif //EQNET_DATAISTREAM_H
