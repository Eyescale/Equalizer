
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

typedef fabric::Segment< Canvas, Segment > Super;

Segment::Segment( Canvas* parent )
        : Super( parent )
        , _channel( 0 )
{
    EQINFO << "New segment @" << (void*)this << std::endl;
}

Segment::Segment( const Segment& from, Canvas* parent )
        : Super( from, parent )
        , _channel( 0 )
{
    Config* config = parent->getConfig();
    EQASSERT( config );

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
        EQASSERT( channel );
        channel->unsetOutput();
    }

    EQASSERT( _destinationChannels.empty( ));
    _destinationChannels.clear();
    _channel = 0;
}

Config* Segment::getConfig()
{
    Canvas* canvas = getCanvas();
    EQASSERT( canvas );
    return canvas ? canvas->getConfig() : 0;
}


const Config* Segment::getConfig() const
{
    const Canvas* canvas = getCanvas();
    EQASSERT( canvas );
    return canvas ? canvas->getConfig() : 0;
}

void Segment::addDestinationChannel( Channel* channel )
{
    EQASSERT( channel );
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
    const Canvas* canvas = getCanvas();
    EQASSERT( canvas );
    SegmentPath path( canvas->getPath( ));
    
    const SegmentVector& segments = canvas->getSegments();
    SegmentVector::const_iterator i = std::find( segments.begin(),
                                                 segments.end(), this );
    EQASSERT( i != segments.end( ));
    path.segmentIndex = std::distance( segments.begin(), i );
    return path;
}
}
}

#include "../lib/fabric/segment.cpp"
template class eq::fabric::Segment< eq::server::Canvas, eq::server::Segment >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
        const eq::fabric::Segment< eq::server::Canvas, eq::server::Segment >& );
/** @endcond */
