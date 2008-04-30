
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNELSTATISTICS_H
#define EQ_CHANNELSTATISTICS_H

#include <eq/client/channelEvent.h>

namespace eq
{
    class Channel;

    /**
     * Holds one statistics event, used for profiling.
     */
    class ChannelStatistics
    {
    public:
        ChannelStatistics( const Statistic::Type type, Channel* channel );
        ~ChannelStatistics();

    private:
        ChannelEvent _event;
    };
}

#endif // EQ_CHANNELSTATISTICS_H
