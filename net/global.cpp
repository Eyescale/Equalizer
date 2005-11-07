
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

#include <eq/base/base.h>
#include <eq/base/log.h>
#include <alloca.h>

using namespace eqNet;
using namespace std;

string Global::_programName;

void Global::setProgramName( const string& programName )
{
    _programName = programName;
    INFO << "Program name set to: " << _programName << endl;
}

const string& Global::getProgramName()
{
    return _programName;
}

