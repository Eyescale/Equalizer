
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

#include <eq/base/base.h>
#include <eq/base/log.h>
#include <alloca.h>

using namespace eqNet;
using namespace std;

string Global::_programName;

void eqNet::init( int argc, char** argv )
{
    ASSERT( argc > 0 );
    
    const string pwd = getenv("PWD");
    Global::setProgramName( pwd + "/" + argv[0] );
}


void Global::setProgramName( const string& programName )
{
    _programName = programName;
    INFO << "Program name set to: " << _programName << endl;
}

const string& Global::getProgramName()
{
    return _programName;
}

