
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_UNI_PIPE_CONNECTION_H
#define EQNET_UNI_PIPE_CONNECTION_H

#include "fdConnection.h"
#include <eq/base/thread.h>

namespace eqNet
{
    /**
     * A uni-directional pipe connection.
     */
    class UniPipeConnection : public FDConnection
    {
    public:
        UniPipeConnection();
        virtual ~UniPipeConnection();

        virtual bool connect( const ConnectionDescription &description );
        virtual void close();

    protected:
        UniPipeConnection( const UniPipeConnection& conn );

    private:
        bool _createPipe();

        int  _pipe[2];
    };
}

#endif //EQNET_UNI_PIPE_CONNECTION_H
