
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "launcher.h"

#include <errno.h>
#include <unistd.h>

using namespace eqNet;
using namespace std;

bool Launcher::run( const std::string& command )
{
    if( command.size() == 0 )
        return false;

    Launcher launcher;
    launcher._command = command;
    launcher.start();
    return true;
}

ssize_t Launcher::run()
{
    INFO << "Executing: " << _command << endl;
    execl( _command.c_str(), NULL );
    WARN << "Error executing '" << _command << "': " << strerror(errno) << endl;
    return EXIT_FAILURE;
}
