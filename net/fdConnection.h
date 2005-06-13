
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_FD_CONNECTION_H
#define EQNET_FD_CONNECTION_H

#include "connection.h"

namespace eqNet
{
    /**
     * A generic file descriptor-based connection, to be subclassed.
     */
    class FDConnection : public Connection
    {
    public:
        virtual size_t read( const void* buffer, const size_t bytes );
        virtual size_t write( const void* buffer, const size_t bytes );

    protected:
        FDConnection();

        int   _readFD;     //!< The read file descriptor.
        int   _writeFD;    //!< The write file descriptor.
    };
};

#endif //EQNET_FD_CONNECTION_H
