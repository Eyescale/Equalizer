/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWEVENT_H
#define EQ_WINDOWEVENT_H

#include <eq/client/event.h>
#include <eq/client/windowSystem.h>

namespace eq
{
    /** A window-system event for an eq::Window */
    class WindowEvent
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
            TYPE_KEY_RELEASE
        };

        Type type;

        union // event data
        {
            ResizeEvent  resize;

            PointerEvent pointerMotion;
            PointerEvent pointerButtonPress;
            PointerEvent pointerButtonRelease;

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
            char fill[128];
        };
    };
}

#endif // EQ_WINDOWEVENT_H

