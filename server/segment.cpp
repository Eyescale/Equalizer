
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "segment.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "view.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataOStream.h>

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

void Segment::getInstanceData( net::DataOStream& os )
{
    // This function is overwritten from eq::Object, since the class is
    // intended to be subclassed on the client side. When serializing a
    // server::Segment, we only transmit the effective bits, not all since that
    // potentially includes bits from subclassed eq::Segments.
    const uint64_t dirty = DIRTY_CUSTOM - 1;
    os << dirty;
    serialize( os, dirty );
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
