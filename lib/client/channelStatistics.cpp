
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channelStatistics.h"

#include "channel.h"
#include "global.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

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
    
    const std::string& name = channel->getName();
    if( name.empty( ))
        snprintf( event.data.statistic.resourceName, 32, "channel %d",
                  channel->getID( ));
    else
        snprintf( event.data.statistic.resourceName, 32, "%s", name.c_str( ));

    if( hint == NICEST && 
        type != Statistic::CHANNEL_COMPRESS && 
        type != Statistic::CHANNEL_TRANSMIT && 
        type != Statistic::CHANNEL_TRANSMIT_NODE )
    {
        channel->getWindow()->finish();
    }
    event.data.statistic.startTime  = channel->getConfig()->getTime();
    event.data.statistic.endTime    = 0.f;
}


ChannelStatistics::~ChannelStatistics()
{
    Channel* channel   = event.channel;
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST && 
        event.data.statistic.type != Statistic::CHANNEL_COMPRESS && 
        event.data.statistic.type != Statistic::CHANNEL_TRANSMIT && 
        event.data.statistic.type != Statistic::CHANNEL_TRANSMIT_NODE )
    {
        channel->getWindow()->finish();
    }
    if( event.data.statistic.endTime == 0.f )
        event.data.statistic.endTime = channel->getConfig()->getTime();
    channel->addStatistic( event );
}

}
