
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "segment.h"

#include "channel.h"
#include "config.h"

using namespace eq::base;

namespace eq
{
namespace server
{

Segment::Segment( const Segment& from, Config* config )
        : eq::Frustum( from )
        , _canvas( 0 )
        , _channel( 0 )
{
    if( from._channel )
    {
        const Channel* oldChannel = from._channel;
        const std::string&   name = oldChannel->getName();
        Channel*       newChannel = config->findChannel( name );
        
        EQASSERT( !name.empty( ));
        EQASSERT( newChannel );
      
        _name = from._name;
        _vp = from._vp;
        
        _channel = newChannel;
    }
}

Segment::~Segment()
{
    _canvas  = 0;
    _channel = 0;
}


std::ostream& operator << ( std::ostream& os, const Segment* segment)
{
    if( !segment )
        return os;
    
    os << disableFlush << disableHeader << "segment" << std::endl;
    os << "{" << std::endl << indent;
    
    const std::string& name = segment->getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    const Channel* channel = segment->getChannel();
    if( channel && !channel->getName().empty( ))
        os << "channel  \"" << channel->getName() << "\"" << std::endl;

    const eq::Viewport& vp  = segment->getViewport();
    if( vp.isValid( ) && vp != eq::Viewport::FULL )
        os << "viewport " << vp << std::endl;

    os << static_cast< const eq::Frustum& >( *segment );

    os << exdent << "}" << std::endl << enableHeader << enableFlush;
    return os;
}

}
}
