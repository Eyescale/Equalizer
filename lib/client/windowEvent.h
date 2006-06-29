/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWEVENT_H
#define EQ_WINDOWEVENT_H

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
            TYPE_MOUSE_MOTION,
            TYPE_MOUSE_BUTTON_PRESS,
            TYPE_MOUSE_BUTTON_RELEASE,
            TYPE_KEY_PRESS,
            TYPE_KEY_RELEASE
        };

        Type type;

        union // event data
        {
            struct
            {
                int32_t x; // relative to screen
                int32_t y;
                int32_t w;
                int32_t h;
            } resize;

            struct
            {
                int32_t x; // relative to window
                int32_t y;
                // TODO button id
            } mouseMotion, mouseButtonPress, mouseButtonRelease;

            struct
            {
                int32_t key; // atm window-system native key code
            } keyPress, keyRelease;
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

