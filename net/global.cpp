
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

#include <eq/base/base.h>
#include <eq/base/log.h>
#include <alloca.h>

using namespace eqNet;
using namespace std;

char* Global::_programName = NULL;

void eqNet::init( int argc, char** argv )
{
    ASSERT( argc > 0 );
    
    Global::setProgramName( argv[0] );
}


void Global::setProgramName( const char* programName )
{
    if( _programName )
        free( _programName );

    _programName = strdup( programName );
    INFO << "Program name set to: " << _programName << endl;
}

const char* Global::getProgramName()
{
    return _programName;
}

