
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CHANNEL_LISTENER_H
#define EQS_CHANNEL_LISTENER_H

#include <eq/base/base.h>

namespace eq
{
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
         * @param startTime the time the channel started executing the frame.
         * @param endTime the time the channel finished the frame.
         */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const float startTime, const float endTime
                                     /*, const float load */ ) = 0;
    };
}
}
#endif // EQS_CHANNEL_LISTENER_H
