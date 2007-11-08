
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "windowSystem.h"

#include <eq/base/debug.h>

using namespace std;

namespace eq
{
EQ_EXPORT void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line )
{                                                                 
    EQWARN << "Got GL error 0x" << hex << error << dec << ' ' << when
           << " in " << file << ':' << line << endl
           << "    Set breakpoint in " << __FILE__ << ':' << __LINE__ 
           << " to debug" << endl;
}                                 
                        
EQ_EXPORT std::ostream& operator << ( std::ostream& os, const WindowSystem ws )
{
    if( ws >= WINDOW_SYSTEM_ALL )
        os << "unknown (" << static_cast<unsigned>( ws ) << ')';
    else 
        os << ( ws == WINDOW_SYSTEM_NONE ? "none" :
                ws == WINDOW_SYSTEM_AGL  ? "agl"  :
                ws == WINDOW_SYSTEM_GLX  ? "glX"  :
                ws == WINDOW_SYSTEM_WGL  ? "wgl"  : "error" );

    return os;
}
}
