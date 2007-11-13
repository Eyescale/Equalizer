
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DATAISTREAM_H
#define EQNET_DATAISTREAM_H

#include <eq/base/buffer.h> // member

#include <iostream>
#include <vector>

namespace eqNet
{
    /**
     * A std::istream for binary data sent with DataOStream.
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
        template< typename T >
        DataIStream& operator >> ( T& value )
            { read( &value, sizeof( value )); return *this; }

        template< typename T >
        DataIStream& operator >> ( std::vector< T >& value )
        {
            uint64_t nElems = 0;
            read( &nElems, sizeof( nElems ));
            value.resize( nElems );
            read( &value[0], nElems * sizeof( T ) ); 
            return *this; 
        }

        void read( void* data, uint64_t size );

        const void*    getRemainingBuffer();
        uint64_t       getRemainingBufferSize();
        void           advanceBuffer( const uint64_t offset ); 

        virtual size_t nRemainingBuffers() const = 0;
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

    // Some template specializations
    template<>
    inline DataIStream& DataIStream::operator >> ( std::string& str )
    { 
        uint64_t nElems = 0;
        read( &nElems, sizeof( nElems ));
        EQASSERT( nElems <= getRemainingBufferSize( ));
        str.assign( static_cast< const char* >(getRemainingBuffer( )), nElems );
        advanceBuffer( nElems );
        return *this; 
    }
}

#endif //EQNET_DATAISTREAM_H
