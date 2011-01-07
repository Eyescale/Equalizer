
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
#include "compound.h"
#include "config.h"
#include "configDestCompoundVisitor.h"
#include "pipe.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <co/dataOStream.h>

namespace eq
{
namespace server
{

typedef fabric::Segment< Canvas, Segment, Channel > Super;

Segment::Segment( Canvas* parent )
        : Super( parent )
{
}

Segment::~Segment()
{
    ConfigDestCompoundVisitor visitor( _destinationChannels,
                                       false /*activeOnly*/ );
    getConfig()->accept( visitor );
    const Compounds& compounds = visitor.getResult();

    for( Compounds::const_iterator i = compounds.begin();
         i != compounds.end(); ++i )
    {
        Compound* compound = *i;
        while( compound )
        {
            Compound* parent = compound->getParent();
            delete compound;
            if( parent && parent->isLeaf( )) // empty parent now
                compound = parent;
            else
                compound = 0;
        }
    }

    // Use copy - Channel::unsetOutput modifies vector
    Channels destinationChannels = _destinationChannels;
    for( Channels::const_iterator i = destinationChannels.begin();
         i != destinationChannels.end(); ++i )
    {
        Channel* channel = *i;
        EQASSERT( channel );
        channel->unsetOutput();
    }

    EQASSERT( _destinationChannels.empty( ));
    _destinationChannels.clear();
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

ServerPtr Segment::getServer() 
{
    Canvas* canvas = getCanvas();
    EQASSERT( canvas );
    return ( canvas ? canvas->getServer() : 0 );
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
    Channels::iterator i = stde::find( _destinationChannels, channel );

    EQASSERT( i !=  _destinationChannels.end( ));
    if( i == _destinationChannels.end( ))
        return false;

    _destinationChannels.erase( i );

    EQASSERT( std::find( _destinationChannels.begin(), 
                         _destinationChannels.end(), channel ) == 
              _destinationChannels.end( ));
    return true;
}

void Segment::findDestinationChannels( const Layout* layout,
                                       Channels& result ) const
{
    for( Channels::const_iterator i = _destinationChannels.begin();
         i != _destinationChannels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->getLayout() == layout )
            result.push_back( channel );
    }
}

SegmentPath Segment::getPath() const
{
    const Canvas* canvas = getCanvas();
    EQASSERT( canvas );
    SegmentPath path( canvas->getPath( ));
    
    const Segments& segments = canvas->getSegments();
    Segments::const_iterator i = std::find( segments.begin(), segments.end(),
                                            this );
    EQASSERT( i != segments.end( ));
    path.segmentIndex = std::distance( segments.begin(), i );
    return path;
}

}
}

#include "../fabric/segment.ipp"
template class eq::fabric::Segment< eq::server::Canvas, eq::server::Segment,
                                    eq::server::Channel >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::server::Super& );
/** @endcond */
