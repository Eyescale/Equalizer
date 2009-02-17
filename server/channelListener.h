
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_LISTENER_H
#define EQS_CHANNEL_LISTENER_H

#include <eq/base/base.h>

namespace eq
{

class Statistic;

namespace server
{
    class Channel;

    /** A listener on various channel operations. */
    class ChannelListener
    {
    public:
        virtual ~ChannelListener(){}

        /** 
         * Notify that the channel has received new load data.
         *
         * @param channel the channel
         * @param frameNumber the frame number.
         * @param nStatistics the statistic's length.
         * @param statistics the frame's statistic.
         */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const uint32_t nStatistics,
                                     const eq::Statistic* statistics )=0;
    };
}
}
#endif // EQS_CHANNEL_LISTENER_H
