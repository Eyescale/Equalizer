
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWEVENT_H
#define EQ_WINDOWEVENT_H

#include <eq/client/event.h>

namespace eq
{
    class Window;

    /** A window-system event for an eq::Window */
    class EQ_EXPORT WindowEvent
    {
    public:
        Window* window;
        Event   data;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream&, const WindowEvent& );
}

#endif // EQ_WINDOWEVENT_H

