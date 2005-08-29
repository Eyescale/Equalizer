
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#include "fdConnection.h"
#include <eq/base/thread.h>

namespace eqNet
{
    /**
     * A bi-directional pipe connection (pair).
     */
    class PipeConnection : public FDConnection
    {
    public:
        PipeConnection();
        virtual ~PipeConnection();

        virtual bool connect( const ConnectionDescription &description );
        virtual void close();

        /** 
         * @return the 'other' end of the pipe connection.
         */
        PipeConnection* getChildEnd(){ return _childConnection; }

    protected:
        PipeConnection( const PipeConnection& conn );

    private:
        bool _createPipes();
        void _setupParent();
        void _setupChild();

        int*            _pipes;
        PipeConnection* _childConnection;
    };
}

#endif //EQNET_PIPE_CONNECTION_H
