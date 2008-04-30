
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
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    _event.channel                    = channel;
    _event.data.type                  = Event::STATISTIC;
    _event.data.originator            = channel->getID();
    _event.data.statistic.type        = type;
    _event.data.statistic.frameNumber = channel->getPipe()->getCurrentFrame();

    if( hint == NICEST )
        channel->getWindow()->finish();
    _event.data.statistic.startTime  = channel->getConfig()->getTime();
}


ChannelStatistics::~ChannelStatistics()
{
    Channel* channel   = _event.channel;
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        channel->getWindow()->finish();

    _event.data.statistic.endTime = channel->getConfig()->getTime();
    channel->addStatistic( _event );
}

}
