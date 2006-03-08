
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_LAUNCHER_H
#define EQNET_LAUNCHER_H

#include <eq/base/thread.h>
#include <string>
#include <vector>

namespace eqNet
{
    /** The launcher executes a shell command in a separate process. */
    class Launcher : public eqBase::Thread
    {
    public:
        static bool run( const std::string& command );

    private:
        Launcher() : eqBase::Thread( eqBase::Thread::FORK ) {}

        std::vector<std::string> _commandLine;

        void _buildCommandLine( const std::string& command );
        virtual ssize_t run();
        
    };
}

#endif // EQNET_LAUNCHER_H
