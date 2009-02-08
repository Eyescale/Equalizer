
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channelStatistics.h"

#include "channel.h"
#include "global.h"
#include "pipe.h"

#ifdef WIN32_VC
#  define snprintf _snprintf
#endif

namespace eq
{

ChannelStatistics::ChannelStatistics( const Statistic::Type type, 
                                      Channel* channel )
        : _channel( channel )
{
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    event.type                  = Event::STATISTIC;
    event.originator            = channel->getID();
    event.statistic.type        = type;
    event.statistic.frameNumber = channel->getPipe()->getCurrentFrame();
    
    const std::string& name = channel->getName();
    if( name.empty( ))
        snprintf( event.statistic.resourceName, 32, "channel %d",
                  channel->getID( ));
    else
        snprintf( event.statistic.resourceName, 32, "%s", name.c_str( ));

    if( hint == NICEST && 
        type != Statistic::CHANNEL_COMPRESS && 
        type != Statistic::CHANNEL_TRANSMIT && 
        type != Statistic::CHANNEL_TRANSMIT_NODE )
    {
        channel->getWindow()->finish();
    }
    event.statistic.startTime  = channel->getConfig()->getTime();
    event.statistic.endTime    = 0;
}


ChannelStatistics::~ChannelStatistics()
{
    const int32_t hint =_channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST && 
        event.statistic.type != Statistic::CHANNEL_COMPRESS && 
        event.statistic.type != Statistic::CHANNEL_TRANSMIT && 
        event.statistic.type != Statistic::CHANNEL_TRANSMIT_NODE )
    {
        _channel->getWindow()->finish();
    }
    if( event.statistic.endTime == 0 )
        event.statistic.endTime = _channel->getConfig()->getTime();

    _channel->addStatistic( event );
}

}
