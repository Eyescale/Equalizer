
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "launcher.h"

#include <errno.h>
#include <unistd.h>

using namespace eqNet;
using namespace std;

void Launcher::run( const char* command )
{
    Launcher launcher;
    launcher._command = command;
    launcher.start();
}

ssize_t Launcher::run()
{
    INFO << "Executing: " << _command << endl;
    execl( _command, NULL );
    WARN << "Error executing '" << _command << "': " << strerror(errno) << endl;
    delete this;
    ::exit( EXIT_FAILURE );
    return EXIT_FAILURE;
}
