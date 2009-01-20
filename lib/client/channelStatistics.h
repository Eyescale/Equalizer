
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNELSTATISTICS_H
#define EQ_CHANNELSTATISTICS_H

#include <eq/client/event.h> // enum Statistic::Type

namespace eq
{
    class Channel;

    /**
     * Holds one statistics event, used for profiling.
     */
    class EQ_EXPORT ChannelStatistics
    {
    public:
        ChannelStatistics( const Statistic::Type type, Channel* channel );
        ~ChannelStatistics();

        Event event;

    private:
        Channel* const _channel;
    };
}

#endif // EQ_CHANNELSTATISTICS_H
