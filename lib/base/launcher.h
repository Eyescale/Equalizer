
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_LAUNCHER_H
#define EQBASE_LAUNCHER_H

#include <eq/base/base.h>
#include <string>
#include <vector>

namespace eqBase
{
    /** The launcher executes a command in a separate process. */
    class Launcher
    {
    public:
        static bool run( const std::string& command );

    private:
        Launcher(){}
#ifndef WIN32
        static void _buildCommandLine( const std::string& command,
                                       std::vector<std::string>& commandLine );
#endif
    };
}

#endif // EQBASE_LAUNCHER_H
