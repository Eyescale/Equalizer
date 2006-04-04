
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_STATE_H
#define EQS_LOADER_STATE_H

namespace eqs
{
    class Loader;
    class Server;
    class Config;
    class Node;
    class Pipe;
    class Window;
    class Channel;
    class Compound;
    class ConnectionDescription;
}

namespace eqLoader
{
    struct State
    {
        State() { reset(); }
        State( eqs::Loader* ldr )
            {
                reset();
                loader = ldr;
            }

        void reset() 
            {
                loader   = NULL;
                server   = NULL;
                config   = NULL;
                node     = NULL;
                pipe     = NULL;
                window   = NULL;
                channel  = NULL;
                compound = NULL;
                connectionDescription = NULL;
            }
        eqs::Loader*      loader;
        eqs::Server*      server;
        eqs::Config*      config;
        eqs::Node*        node;
        eqs::Pipe*        pipe;
        eqs::Window*      window;
        eqs::Channel*     channel;
        eqs::Compound*    compound;
        eqs::ConnectionDescription* connectionDescription;
    };
}


#endif // EQS_LOADER_STATE_H
