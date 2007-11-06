
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "windowSystem.h"

#include <eq/base/debug.h>

using namespace std;

namespace eq
{
void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line )
{                                                                 
    EQWARN << "Got GL error 0x" << hex << error << dec << ' ' << when
           << " in " << file << ':' << line << endl
           << "    Set breapoint in " << __FILE__ << ':' << __LINE__ 
           << " to debug" << endl;
}                                 
                        
}
