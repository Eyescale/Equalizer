
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_PIPE_CONNECTION_H
#define EQNET_PIPE_CONNECTION_H

#include "fdConnection.h"

namespace eqNet
{
    namespace internal
    {
        /**
         * A fork-based pipe connection.
         */
        class PipeConnection : public FDConnection
        {
        public:
            PipeConnection();
            virtual ~PipeConnection();

            virtual bool connect( ConnectionDescription &description );
            virtual void close();

        protected:

        private:
            void _createPipes();

            void _setupParent();
            void _runChild( const char *entryFunc );

            int *_pipes;
        };
    }
}

#endif //EQNET_PIPE_CONNECTION_H
