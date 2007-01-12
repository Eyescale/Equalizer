
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#include "fdConnection.h"
#include <eq/base/thread.h>

namespace eqNet
{
    /**
     * A uni-directional pipe connection.
     */
    class PipeConnection : public FDConnection
    {
    public:
        PipeConnection();
        virtual ~PipeConnection();

        virtual bool connect();
        virtual void close();

    protected:
        PipeConnection( const PipeConnection& conn );

    private:
        bool _createPipe();

        int  _pipe[2];
    };
}

#endif //EQNET_PIPE_CONNECTION_H
