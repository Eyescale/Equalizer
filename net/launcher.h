
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_LAUNCHER_H
#define EQNET_LAUNCHER_H

#include <eq/base/thread.h>

namespace eqNet
{
    /** The launcher executes a shell command in a separate process. */
    class Launcher : public eqBase::Thread
    {
    public:
        static void run( const char* command );

    private:
        Launcher() : eqBase::Thread( eqBase::Thread::FORK ) {}
        virtual ssize_t run();
        
        const char* _command;
    };
}

#endif // EQNET_LAUNCHER_H
