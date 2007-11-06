
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#ifdef WIN32
#  include <eq/net/connection.h>
#else
#  include <eq/net/fdConnection.h>
#endif

#include <eq/base/thread.h>

namespace eqNet
{
    /**
     * A uni-directional pipe connection.
     */
    class PipeConnection 
#ifdef WIN32
        : public Connection
#else
        : public FDConnection
#endif
    {
    public:
        PipeConnection();
        virtual ~PipeConnection();

        virtual bool connect();
        virtual void close();

#ifdef WIN32
        virtual ReadNotifier getReadNotifier() { return _dataPending; }
#endif

    protected:
        PipeConnection( const PipeConnection& conn );

#ifdef WIN32
        virtual int64_t read( void* buffer, const uint64_t bytes );
        virtual int64_t write( const void* buffer, const uint64_t bytes ) const;
#endif

    private:
        bool _createPipe();

#ifdef WIN32
        HANDLE _readHandle;
        HANDLE _writeHandle;
        mutable eqBase::Lock _mutex;
        mutable uint64_t     _size;
        mutable HANDLE       _dataPending;
#endif
    };
}

#endif //EQNET_PIPE_CONNECTION_H
