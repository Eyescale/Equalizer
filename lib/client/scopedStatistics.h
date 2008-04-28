
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_SCOPEDSTATISTICS_H
#define EQ_SCOPEDSTATISTICS_H

#include <eq/client/channelEvent.h>

namespace eq
{
    class Channel;

    /**
     * Holds one statistics event, used for profiling.
     */
    class ScopedStatistics
    {
    public:
        ScopedStatistics( const Statistic::Type type, Channel* channel );
        ~ScopedStatistics();

    private:
        ChannelEvent _event;
    };
}

#endif // EQ_SCOPEDSTATISTICS_H
