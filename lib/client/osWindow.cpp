/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#include "osWindow.h"

#ifdef AGL
#  include "aglWindow.h"
#endif
#ifdef GLX
#  include "glXWindow.h"
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

        default:
            EQASSERTINFO( 0, "Not Implemented" );
    }
    EQERROR << "failed to create window-system specific window " 
            << pipe->getWindowSystem() << std::endl;

    return 0;
}

}
