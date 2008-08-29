
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLWINDOWEVENT_H
#define EQ_AGLWINDOWEVENT_H

#include <eq/client/event.h>

namespace eq
{
    /** A window-system event for an AGLWindowIF */
    class EQ_EXPORT AGLWindowEvent : public Event
    {
    public:
        // Native event
        EventRef carbonEventRef;
    };
}

#endif // EQ_AGLWINDOWEVENT_H

