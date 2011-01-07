
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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
#include "configDestCompoundVisitor.h"
#include "layout.h"
#include "log.h"
#include "node.h"
#include "pipe.h"
#include "segment.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/base/stdExt.h>

namespace eq
{
namespace server
{
typedef fabric::Canvas< Config, Canvas, Segment, Layout > Super;

Canvas::Canvas( Config* parent )
        : Super( parent )
        , _state( STATE_STOPPED )
{}

Canvas::~Canvas()
{
}

Segment* Canvas::getSegment( const SegmentPath& path )
{
    const Segments& segments = getSegments();
    EQASSERTINFO( segments.size() > path.segmentIndex,
                  segments.size() << " <= " << path.segmentIndex );

    if( segments.size() <= path.segmentIndex )
        return 0;

    return segments[ path.segmentIndex ];
}

ServerPtr Canvas::getServer() 
{
    Config* config = getConfig();
    EQASSERT( config );
    return ( config ? config->getServer() : 0 );
}

CanvasPath Canvas::getPath() const
{
    const Config* config = getConfig();
    EQASSERT( config );

    const Canvases& canvases = config->getCanvases();
    Canvases::const_iterator i = std::find( canvases.begin(), canvases.end(),
                                            this );
    EQASSERT( i != canvases.end( ));

    CanvasPath path;
    path.canvasIndex = std::distance( canvases.begin(), i );
    return path;
}

void Canvas::activateLayout( const uint32_t index )
{
    if( _state == STATE_RUNNING )
        _switchLayout( getActiveLayoutIndex(), index );
}

void Canvas::init()
{
    EQASSERT( _state == STATE_STOPPED );
    _switchLayout( EQ_UNDEFINED_UINT32, getActiveLayoutIndex( ));
    _state = STATE_RUNNING;
}

void Canvas::exit()
{
    EQASSERT( _state == STATE_RUNNING || _state == STATE_DELETE );
    _switchLayout( getActiveLayoutIndex(), EQ_UNDEFINED_UINT32 );
    if( _state == STATE_RUNNING )
        _state = STATE_STOPPED;
}

void Canvas::_switchLayout( const uint32_t oldIndex, const uint32_t newIndex )
{
    if( oldIndex == newIndex )
        return;

    const Layouts& layouts = getLayouts();
    const size_t nLayouts = layouts.size();
    Layout* oldLayout = (oldIndex >= nLayouts) ? 0 : layouts[oldIndex];
    Layout* newLayout = (newIndex >= nLayouts) ? 0 : layouts[newIndex];

    if( oldLayout )
        oldLayout->trigger( this, false );

    if( newIndex == EQ_UNDEFINED_UINT32 )
        return;

    Super::activateLayout( newIndex );

    if( newLayout )
        newLayout->trigger( this, true );
}

void Canvas::postDelete()
{
    _state = STATE_DELETE;
    getConfig()->postNeedsFinish();
}

}
}

#include "nodeFactory.h"
#include "../fabric/canvas.ipp"

template class eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                                   eq::server::Segment, eq::server::Layout >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
    const eq::fabric::Canvas< eq::server::Config, eq::server::Canvas,
                              eq::server::Segment, eq::server::Layout >& );
/** @endcond */
