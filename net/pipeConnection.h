
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#include "fdConnection.h"
#include <eq/base/thread.h>

namespace eqNet
{
    namespace priv
    {
        /**
         * A fork-based pipe connection.
         */
        class PipeConnection : public FDConnection, public eqBase::Thread
        {
        public:
            PipeConnection();
            virtual ~PipeConnection();

            virtual bool connect( const ConnectionDescription &description );
            virtual void close();

        protected:
            virtual ssize_t run();

        private:
            void _createPipes();

            void _setupParent();

            int*            _pipes;
            const char*     _entryFunc;
        };
    }
}

#endif //EQNET_PIPE_CONNECTION_H
