
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSTATISTICS_H
#define EQ_WINDOWSTATISTICS_H

#include <eq/client/windowEvent.h>

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
        WindowEvent _event;
    };
}

#endif // EQ_WINDOWSTATISTICS_H
