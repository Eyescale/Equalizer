
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

    protected:
        virtual ~BufferConnection();

        virtual int64_t write( const void* buffer, const uint64_t bytes ) const;
        void sendBuffer( eqBase::RefPtr<Connection> connection );

    private:
        mutable uint8_t* _buffer;
        mutable uint64_t _size;
        mutable uint64_t _maxSize;
    };
}

#endif //EQNET_BUFFER_CONNECTION_H
