
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLXWINDOWEVENT_H
#define EQ_GLXWINDOWEVENT_H

#include <eq/client/event.h>

namespace eq
{
    /** A window-system event for a GLXWindowIF */
    class EQ_EXPORT GLXWindowEvent : public Event
    {
    public:
        // Native event
        XEvent xEvent;
    };
}

#endif // EQ_GLXWINDOWEVENT_H

