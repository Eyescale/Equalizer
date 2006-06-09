
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#include "fdConnection.h"

#include <eq/base/refPtr.h>
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

        virtual bool connect();
        virtual void close();

        /** 
         * @return the 'other' end of the pipe connection.
         */
        eqBase::RefPtr<Connection> getChildEnd(){ return _childConnection; }

    protected:
        PipeConnection( const PipeConnection& conn );
        virtual ~PipeConnection();

    private:
        bool _createPipes();
        void _setupParent();
        void _setupChild();

        int*                       _pipes;
        eqBase::RefPtr<Connection> _childConnection;
    };
}

#endif //EQNET_PIPE_CONNECTION_H
