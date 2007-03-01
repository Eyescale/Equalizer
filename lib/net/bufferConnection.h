
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BUFFER_CONNECTION_H
#define EQNET_BUFFER_CONNECTION_H

#include <eq/net/connection.h> // base class

namespace eqNet
{
    /**
     * A proxy connection buffering outgoing data into a memory region.
     */
    class BufferConnection : public Connection
    {
    public:
        BufferConnection();
        BufferConnection( const BufferConnection& from );
        virtual ~BufferConnection();

        void sendBuffer( eqBase::RefPtr<Connection> connection );
        void swap( BufferConnection& connection );

        uint64_t getSize() const { return _size; }

    protected:
        virtual int64_t read( void* buffer, const uint64_t bytes )
            { EQDONTCALL; return -1; }
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const;

    private:
        mutable uint8_t* _buffer;
        mutable uint64_t _size;
        mutable uint64_t _maxSize;
    };
}

#endif //EQNET_BUFFER_CONNECTION_H
