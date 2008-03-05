
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "statEvent.h"

#include "channel.h"
#include "global.h"

namespace eq
{
EQ_EXPORT std::string StatEvent::_typeNames[TYPE_ALL] = 
{
    std::string( "NO EVENT          " ),
    std::string( "channel clear     " ),
    std::string( "channel draw      " ),
    std::string( "channel finishdraw" ),
    std::string( "channel assemble  " ),
    std::string( "channel readback  " ),
    std::string( "channel transmit  " ),
    std::string( "channel transmit 1" ),
    std::string( "channel wait frame" )
};

StatEvent::StatEvent( const Type type, Channel* channel )
        : data( type, channel->getID( ))
        , _channel( channel ) 
{
    const int32_t hint = channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        channel->getWindow()->finish();

    data.startTime = channel->getPipe()->getFrameTime();
}


StatEvent::~StatEvent()
{
    const int32_t hint =_channel->getIAttribute(Channel::IATTR_HINT_STATISTICS);
    if( hint == OFF )
        return;

    if( hint == NICEST )
        _channel->getWindow()->finish();

    data.endTime = _channel->getPipe()->getFrameTime();
    _channel->addStatEvent( data );
}

}
