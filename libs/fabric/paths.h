
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

#ifndef EQFABRIC_PATHS_H
#define EQFABRIC_PATHS_H

#include "types.h"

/** @cond IGNORE */
namespace eq
{
namespace fabric
{

//----- defines path types with are used to reference entities
// node...channel hierarchy
struct NodePath
{
    NodePath( const uint32_t index = 0 ) : nodeIndex( index ) {}
    uint32_t nodeIndex;
};

struct PipePath : public NodePath
{
    PipePath( const uint32_t index = 0 ) : pipeIndex( index ) {}
    PipePath( const NodePath& p ) : NodePath( p ), pipeIndex( 0 ) {}
    uint32_t pipeIndex;
};

struct WindowPath : public PipePath
{
    WindowPath( const uint32_t index = 0 ) : windowIndex( index ) {}
    WindowPath( const PipePath& p ) : PipePath( p ), windowIndex( 0 ) {}
    uint32_t windowIndex;
};

struct ChannelPath : public WindowPath
{
    ChannelPath( const uint32_t index = 0 ) : channelIndex( index ) {}
    ChannelPath( const WindowPath& p ) : WindowPath( p ), channelIndex( 0 ) {}
    uint32_t channelIndex;
};

// View hierarchy
struct CanvasPath
{
    CanvasPath( const uint32_t index = 0 ) : canvasIndex( index ) {}
    uint32_t canvasIndex;
};

struct SegmentPath : public CanvasPath
{
    SegmentPath( const uint32_t index = 0 ) : segmentIndex( index ) {}
    SegmentPath( const CanvasPath& p ) : CanvasPath( p ), segmentIndex( 0 ) {}
    uint32_t segmentIndex;
};

struct ObserverPath
{
    ObserverPath( const uint32_t index = 0 ) : observerIndex( index ) {}
    uint32_t observerIndex;
};

struct LayoutPath
{
    LayoutPath( const uint32_t index = 0 ) : layoutIndex( index ) {}
    uint32_t layoutIndex;
};

struct ViewPath : public LayoutPath
{
    ViewPath( const uint32_t index = 0 ) : viewIndex( index ) {}
    ViewPath( const LayoutPath& p ) : LayoutPath( p ), viewIndex( 0 ) {}
    uint32_t viewIndex;
};

// ostream operators
inline std::ostream& operator << ( std::ostream& os, const NodePath& path )
{
    os << "node " << path.nodeIndex;
    return os;
}
inline std::ostream& operator << ( std::ostream& os, const PipePath& path )
{
    os << static_cast< const NodePath& >( path ) << " pipe " << path.pipeIndex;
    return os;
}
inline std::ostream& operator << ( std::ostream& os, const WindowPath& path )
{
    os << static_cast< const PipePath& >( path ) << " window "
       << path.windowIndex;
    return os;
}
inline std::ostream& operator << ( std::ostream& os, const ChannelPath& path )
{
    os << static_cast< const WindowPath& >( path ) << " channel "
       << path.channelIndex;
    return os;
}

inline std::ostream& operator << ( std::ostream& os, const ObserverPath& path )
{
    os << "observer " << path.observerIndex;
    return os;
}

inline std::ostream& operator << ( std::ostream& os, const LayoutPath& path )
{
    os << "layout   " << path.layoutIndex;
    return os;
}
inline std::ostream& operator << ( std::ostream& os, const ViewPath& path )
{
    os << static_cast< const LayoutPath& >( path ) << " view "
       << path.viewIndex;
    return os;
}

inline std::ostream& operator << ( std::ostream& os, const CanvasPath& path )
{
    os << "canvas " << path.canvasIndex;
    return os;
}
inline std::ostream& operator << ( std::ostream& os, const SegmentPath& path )
{
    os << static_cast< const CanvasPath& >( path ) << " segment "
       << path.segmentIndex;
    return os;
}
/** @endcond */

}
}
#endif // EQFABRIC_PATHS_H
