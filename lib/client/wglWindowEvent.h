
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLWINDOWEVENT_H
#define EQ_WGLWINDOWEVENT_H

#include <eq/client/event.h>        // base class
#include <eq/client/windowSystem.h> // windows.h

namespace eq
{
    /** A window-system event for a WGLWindowIF */
    class EQ_EXPORT WGLWindowEvent : public Event
    {
    public:
        // Native event data
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
    };
}

#endif // EQ_WGLWINDOWEVENT_H

