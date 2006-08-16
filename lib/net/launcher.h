
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_LAUNCHER_H
#define EQNET_LAUNCHER_H

#include <eq/base/process.h>

#include <string>
#include <vector>

namespace eqNet
{
    /** The launcher executes a shell command in a separate process. */
    class Launcher : public eqBase::Process
    {
    public:
        static bool run( const std::string& command );

    private:
        Launcher(){}

        std::vector<std::string> _commandLine;

        void _buildCommandLine( const std::string& command );
        virtual int run();
        
    };
}

#endif // EQNET_LAUNCHER_H
