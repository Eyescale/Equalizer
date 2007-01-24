/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWEVENT_H
#define EQ_WINDOWEVENT_H

#include <eq/client/event.h>
#include <eq/client/windowSystem.h>

namespace eq
{
    class Window;

    /** A window-system event for an eq::Window */
    class EQ_EXPORT WindowEvent
    {
    public:
        enum Type
        {
            TYPE_EXPOSE,
            TYPE_RESIZE,
            TYPE_POINTER_MOTION,
            TYPE_POINTER_BUTTON_PRESS,
            TYPE_POINTER_BUTTON_RELEASE,
            TYPE_KEY_PRESS,
            TYPE_KEY_RELEASE,
            TYPE_UNHANDLED
        };

        Type    type;
        Window* window;

        union // event data
        {
            ResizeEvent  resize;

            PointerEvent pointerEvent;
            PointerEvent pointerMotion;
            PointerEvent pointerButtonPress;
            PointerEvent pointerButtonRelease;

            KeyEvent     keyEvent;
            KeyEvent     keyPress;
            KeyEvent     keyRelease;
        };

        union // Native event
        {
#ifdef GLX
            XEvent xEvent; // 96 bytes
#endif
#ifdef CGL
            // TODO
#endif
#ifdef WGL
            struct
            {
                HWND hWnd;
                UINT uMsg;
                WPARAM wParam;
                LPARAM lParam;
            };
#endif
            char fill[128];
        };
    };
}

#endif // EQ_WINDOWEVENT_H

