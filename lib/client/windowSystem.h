
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

namespace eq
{
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_CGL,
        WINDOW_SYSTEM_ALL      // must be last
    };
}

#endif // EQ_WINDOWSYSTEM_H

