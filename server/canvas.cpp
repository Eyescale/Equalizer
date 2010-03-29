
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

#include "canvas.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "layout.h"
#include "log.h"
#include "nameFinder.h"
#include "node.h"
#include "pipe.h"
#include "segment.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>


namespace eq
{
namespace server
{
typedef fabric::Canvas< Config, Canvas, Segment, Layout > Super;

Canvas::Canvas( Config* parent )
        : Super( parent )
{}

Canvas::Canvas( const Canvas& from, Config* parent )
        : Super( from, parent )
{
}

Canvas::~Canvas()
{
}

Segment* Canvas::getSegment( const SegmentPath& path )
{
    const SegmentVector& segments = getSegments();
    EQASSERTINFO( segments.size() > path.segmentIndex,
                  segments.size() << " <= " << path.segmentIndex );

    if( segments.size() <= path.segmentIndex )
        return 0;

    return segments[ path.segmentIndex ];
}

CanvasPath Canvas::getPath() const
{
    const Config* config = getConfig();
    EQASSERT( config );

    const CanvasVector& canvases = config->getCanvases();
    CanvasVector::const_iterator i = std::find( canvases.begin(),
                                                 canvases.end(), this );
    EQASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

Segment* Canvas::findSegment( const std::string& name )
{
    SegmentFinder finder( name );
    accept( finder );
    return finder.getResult();
}

void Canvas::activateLayout( const uint32_t index )
{
    const Config* config = getConfig();
    if( config && config->isRunning( ))
        _switchLayout( getActiveLayoutIndex(), index );
}

Segment* Canvas::createSegment()
{
    return new Segment( this );
}

void Canvas::releaseSegment( Segment* segment )
{
    delete segment;
}

void Canvas::init()
{
    _switchLayout( EQ_ID_NONE, getActiveLayoutIndex( ));
}

void Canvas::exit()
{
    _switchLayout( getActiveLayoutIndex(), EQ_ID_NONE );
}

namespace
{
class ActivateVisitor : public ConfigVisitor
{
public:
    ActivateVisitor( const ChannelVector& channels ) : _channels( channels ) {}
    virtual ~ActivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->activate();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    ChannelVector _channels;
};

class DeactivateVisitor : public ConfigVisitor
{
public:
    DeactivateVisitor( ChannelVector& channels )
            : _channels( channels ) {}
    virtual ~DeactivateVisitor() {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel* channel = compound->getChannel();
            if( !channel )
                return TRAVERSE_CONTINUE;
            
            for( ChannelVector::iterator i = _channels.begin();
                 i != _channels.end(); ++i )
            {
                Channel* destChannel = *i;
                if( destChannel != channel ) 
                    continue;

                compound->deactivate();
                break;
            }

            return TRAVERSE_PRUNE;
        }

private:
    ChannelVector& _channels;
};
}

void Canvas::_switchLayout( const uint32_t oldIndex, const uint32_t newIndex )
{
    Config* config = getConfig();
    EQASSERT( config );
    if( oldIndex == newIndex )
        return;

    const LayoutVector& layouts = getLayouts();
    const size_t nLayouts = layouts.size();
    const Layout* oldLayout = (oldIndex >= nLayouts) ? 0 : layouts[oldIndex];
    const Layout* newLayout = (newIndex >= nLayouts) ? 0 : layouts[newIndex];

    const SegmentVector& segments = getSegments();
    for( SegmentVector::const_iterator i = segments.begin();
         i != segments.end(); ++i )
    {
        const Segment* segment = *i;        
        const ChannelVector& destChannels = segment->getDestinationChannels();

        if( newLayout )
        {
            // activate channels used by new layout
            ChannelVector usedChannels;
            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == newLayout )
                    usedChannels.push_back( channel );
            }
            
            ActivateVisitor activator( usedChannels );
            config->accept( activator );
        }

        if( oldLayout )
        {
            // de-activate channels used by old layout
            ChannelVector usedChannels;

            for( ChannelVector::const_iterator j = destChannels.begin();
                 j != destChannels.end(); ++j )
            {
                Channel*       channel       = *j;
                const Layout*  channelLayout = channel->getLayout();
                if( channelLayout == oldLayout )
                    usedChannels.push_back( channel );
            }
            DeactivateVisitor deactivator( usedChannels );
            config->accept( deactivator );
        }
    }
}

void Canvas::deregister()
{
    net::Session* session = getSession();
    EQASSERT( session );

    const SegmentVector& segments = getSegments();
    for( SegmentVector::const_iterator i = segments.begin(); 
         i != segments.end(); ++i )
    {
        Segment* segment = *i;
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        EQASSERT( segment->isMaster( ));
        
        session->deregisterObject( segment );
    }

    EQASSERT( getID() != EQ_ID_INVALID );
    EQASSERT( isMaster( ));
    session->deregisterObject( this );
}


}
}

#include "../lib/fabric/canvas.cpp"
template class eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                                   eq::server::Segment, eq::server::Layout >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
    const eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                              eq::server::Segment, eq::server::Layout >& );
/** @endcond */
