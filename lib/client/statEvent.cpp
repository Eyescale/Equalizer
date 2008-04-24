
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "statEvent.h"

#include "channel.h"
#include "global.h"

namespace eq
{

ScopedStatistics::ScopedStatistics( const StatEvent::Type type, Channel* channel )
        : _channel( channel )
{
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    _event.type       = type;

    if( hint == NICEST )
        channel->getWindow()->finish();
    _event.startTime  = channel->getPipe()->getFrameTime();
}


ScopedStatistics::~ScopedStatistics()
{
    const int32_t hint =_channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        _channel->getWindow()->finish();

    _event.endTime = _channel->getPipe()->getFrameTime();
    _channel->addStatEvent( _event );
}

}
