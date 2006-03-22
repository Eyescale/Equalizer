
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_STATE_H
#define EQS_LOADER_STATE_H

namespace eqLoader
{
    struct State
    {
        State( Loader* ldr )
                : loader( ldr ),
                  server( NULL ),
                  config( NULL ),
                  node( NULL ),
                  pipe( NULL ),
                  window( NULL ),
                  channel( NULL ),
                  compound( NULL )
            {}

        Loader*   loader;
        Server*   server;
        Config*   config;
        Node*     node;
        Pipe*     pipe;
        Window*   window;
        Channel*  channel;
        Compound* compound;
    };
}


#endif // EQS_LOADER_STATE_H
