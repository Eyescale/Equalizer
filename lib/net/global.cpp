
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

using namespace eqNet;

std::string Global::_programName;
std::string Global::_workDir;
uint16_t    Global::_defaultPort = 0;

void Global::setProgramName( const std::string& programName )
{
    _programName = programName;
}
void Global::setWorkDir( const std::string& workDir )
{
    _workDir = workDir; 
}
