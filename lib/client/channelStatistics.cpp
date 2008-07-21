
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channelStatistics.h"

#include "channel.h"
#include "global.h"

namespace eq
{

ChannelStatistics::ChannelStatistics( const Statistic::Type type, 
                                      Channel* channel )
{
    event.channel                    = channel;

    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    event.data.type                  = Event::STATISTIC;
    event.data.originator            = channel->getID();
    event.data.statistic.type        = type;
    event.data.statistic.frameNumber = channel->getPipe()->getCurrentFrame();

    if( hint == NICEST )
        channel->getWindow()->finish();
    event.data.statistic.startTime  = channel->getConfig()->getTime();
    event.data.statistic.endTime    = 0.f;
}


ChannelStatistics::~ChannelStatistics()
{
    Channel* channel   = event.channel;
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        channel->getWindow()->finish();

    if( event.data.statistic.endTime == 0.f )
        event.data.statistic.endTime = channel->getConfig()->getTime();
    channel->addStatistic( event );
}

}
