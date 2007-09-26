
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
    class EQ_EXPORT DataIStream : public std::istream
    {
    public:
        DataIStream();
        virtual ~DataIStream();

        /** @name Basic data input */
        //*{
        DataIStream& operator >> ( int& value )
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
        //*}
 
    protected:
        virtual bool getNextBuffer( const void** buffer, uint64_t* size ) = 0;

    private:
        /** The current input buffer */
        const char* _input;
        /** The size of the input buffer */
        uint64_t _inputSize;
        /** The current read position in the buffer */
        uint64_t  _position;
    };
}
#endif //EQNET_DATAISTREAM_H
