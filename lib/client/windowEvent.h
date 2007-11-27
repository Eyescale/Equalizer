
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
            EXPOSE = 0,
            RESIZE,
            POINTER_MOTION,
            POINTER_BUTTON_PRESS,
            POINTER_BUTTON_RELEASE,
            KEY_PRESS,
            KEY_RELEASE,
            CLOSE,
            UNHANDLED,
            ALL // must be last
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
#ifdef AGL
            EventRef carbonEventRef;
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

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const WindowEvent& event );
    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const WindowEvent::Type type );
}

#endif // EQ_WINDOWEVENT_H

