
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "segment.h"

#include "channel.h"
#include "config.h"

namespace eq
{
namespace server
{

Segment::Segment( const Segment& from, Config* config )
        : _canvas( 0 ),
          _channel( 0 )
{
    if( from._channel )
    {
        const Channel* oldChannel = from._channel;
        const std::string&   name = oldChannel->getName();
        Channel*       newChannel = config->findChannel( name );

        EQASSERT( !name.empty( ));
        EQASSERT( newChannel );
            
        _channel = newChannel;
    }
}

Segment::~Segment()
{
    _canvas  = 0;
    _channel = 0;
}

void Segment::setWall( const eq::Wall& wall )
{
    //eq::Segment::setWall( wall );
    //_updateSegment();
}
        
void Segment::setProjection( const eq::Projection& projection )
{
    //eq::Segment::setProjection( projection );
    //_updateSegment();
}


}
}
