
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "segment.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "paths.h"
#include "view.h"

using namespace eq::base;

namespace eq
{
namespace server
{

Segment::Segment()
        : _canvas( 0 )
        , _channel( 0 )
{
    EQINFO << "New segment @" << (void*)this << std::endl;
}

Segment::Segment( const Segment& from, Config* config )
        : eq::Segment( from )
        , _canvas( 0 )
        , _channel( 0 )
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
        const ChannelPath channelPath( oldChannel->getPath( ));

        Channel* newChannel = config->getChannel( channelPath );
        EQASSERT( newChannel );

        const View* oldView = oldChannel->getView();
        EQASSERT( oldView );
        const ViewPath viewPath( oldView->getPath( ));

        View* newView = config->getView( viewPath );
        EQASSERT( newView );

        newChannel->setOutput( newView, this );
    }

    EQINFO << "Copy segment @" << (void*)this << std::endl;
}

Segment::~Segment()
{
    EQINFO << "Delete segment @" << (void*)this << std::endl;

    // Use copy - Channel::unsetOutput modifies vector
    ChannelVector destinationChannels = _destinationChannels;
    for( ChannelVector::const_iterator i = destinationChannels.begin();
         i != destinationChannels.end(); ++i )
    {
        Channel* channel = *i;
        channel->unsetOutput();
    }

    EQASSERT( _destinationChannels.empty( ));
    _destinationChannels.clear();
    _channel = 0;
    _canvas  = 0;
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

void Segment::setViewport( const eq::Viewport& vp ) 
{
    _vp = vp; 
    setDirty( DIRTY_VIEWPORT );
}

void Segment::addDestinationChannel( Channel* channel )
{
    EQASSERT( std::find( _destinationChannels.begin(), 
                         _destinationChannels.end(), channel ) == 
              _destinationChannels.end( ));

    _destinationChannels.push_back( channel );
}

bool Segment::removeDestinationChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _destinationChannels.begin(), 
                                      _destinationChannels.end(), channel );

    EQASSERT( i !=  _destinationChannels.end( ));
    if( i == _destinationChannels.end( ))
        return false;

    _destinationChannels.erase( i );

    EQASSERT( std::find( _destinationChannels.begin(), 
                         _destinationChannels.end(), channel ) == 
              _destinationChannels.end( ));
    return true;
}

SegmentPath Segment::getPath() const
{
    EQASSERT( _canvas );
    SegmentPath path( _canvas->getPath( ));
    
    const SegmentVector&   segments = _canvas->getSegments();
    SegmentVector::const_iterator i = std::find( segments.begin(),
                                                 segments.end(), this );
    EQASSERT( i != segments.end( ));
    path.segmentIndex = std::distance( segments.begin(), i );
    return path;
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
