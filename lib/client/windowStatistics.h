
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSTATISTICS_H
#define EQ_WINDOWSTATISTICS_H

#include <eq/client/event.h>

namespace eq
{
    class Window;

    /**
     * Holds one statistics event, used for profiling.
     */
    class WindowStatistics
    {
    public:
        WindowStatistics( const Statistic::Type type, Window* window );
        ~WindowStatistics();

    private:
        Event         _event;
        Window* const _window;
    };
}

#endif // EQ_WINDOWSTATISTICS_H
