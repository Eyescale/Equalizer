/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#include "OSWindow.h"

#ifdef AGL
#  include "aglWindow.h"
#endif
#ifdef GLX
#  include "glxWindow.h"
#endif
#ifdef WGL
#  include "wglWindow.h"
#endif

namespace eq
{

OSWindow* OSWindow::createOSWindow( Window* parent )
{
    const Pipe* pipe = parent->getPipe();

    switch( pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
            EQINFO << "Making new GLX" << std::endl;
            return new GLXWindow( parent );
#endif
            break;

        case WINDOW_SYSTEM_AGL:
#ifdef AGL
            EQINFO << "Making new AGL" << std::endl;
            return new AGLWindow( parent );
#endif
            break;

        case WINDOW_SYSTEM_WGL:
#ifdef WGL
            EQINFO << "Making new WGL" << std::endl;
            return new WGLWindow( parent );
#endif
            break;
    }
    EQERROR << "failed to create window specific system " 
            << pipe->getWindowSystem() << std::endl;

    return 0;
}

}
