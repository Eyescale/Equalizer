
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "segment.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "paths.h"

using namespace eq::base;

namespace eq
{
namespace server
{

Segment::Segment()
        : _canvas( 0 )
        , _channel( 0 )
{
}

Segment::Segment( const Segment& from, Config* config )
        : eq::Frustum( from )
        , _canvas( 0 )
        , _channel( 0 )
        , _vp( from._vp )
{
    if( from._channel )
    {
        const Channel* oldChannel = from._channel;
        const ChannelPath path( oldChannel->getPath( ));

        _channel = config->getChannel( path );
        EQASSERT( _channel );
    }
    for( ChannelVector::const_iterator i = from._destinationChannels.begin();
         i != from._destinationChannels.end(); ++i )
    {
        const Channel* oldChannel = *i;
        const ChannelPath path( oldChannel->getPath( ));

        Channel* newChannel = getConfig()->getChannel( path );
        EQASSERT( newChannel );

        addDestinationChannel( newChannel );
    }
}

Segment::~Segment()
{
    _canvas  = 0;
    _channel = 0;
}

Config* Segment::getConfig()
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}


const Config* Segment::getConfig() const
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}

void Segment::addDestinationChannel( Channel* channel )
{
    _destinationChannels.push_back( channel );
    channel->setSegment( this );
}

bool Segment::removeDestinationChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _destinationChannels.begin(), 
                                      _destinationChannels.end(), channel );

    if( i == _destinationChannels.end( ))
        return false;

    channel->setSegment( 0 );
    _destinationChannels.erase( i );
    return true;
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
