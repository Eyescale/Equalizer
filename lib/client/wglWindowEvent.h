
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLWINDOWEVENT_H
#define EQ_WGLWINDOWEVENT_H

#include <eq/client/windowEvent.h>
#include <eq/client/windowSystem.h>

namespace eq
{
    /** A window-system event for a WGLWindowIF */
    class EQ_EXPORT WGLWindowEvent : public WindowEvent
    {
    public:
        // Native event data
        HWND hWnd;
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
    };
}

#endif // EQ_WGLWINDOWEVENT_H

