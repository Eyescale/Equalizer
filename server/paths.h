
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_PATHS_H
#define EQSERVER_PATHS_H

#include "types.h"

namespace eq
{
namespace server
{

//----- defines path types with are used to reference entities
// node...channel hierarchy
struct NodePath
{
    NodePath() : nodeIndex( 0 ) {}
    uint32_t nodeIndex;
};

struct PipePath : public NodePath
{
    PipePath() : pipeIndex( 0 ) {}
    PipePath( const NodePath& p ) : NodePath( p ), pipeIndex( 0 ) {}
    uint32_t pipeIndex;
};

struct WindowPath : public PipePath
{
    WindowPath() : windowIndex( 0 ) {}
    WindowPath( const PipePath& p ) : PipePath( p ), windowIndex( 0 ) {}
    uint32_t windowIndex;
};

struct ChannelPath : public WindowPath
{
    ChannelPath() : channelIndex( 0 ) {}
    ChannelPath( const WindowPath& p ) : WindowPath( p ), channelIndex( 0 ) {}
    uint32_t channelIndex;
};

// View hierarchy
struct CanvasPath
{
    CanvasPath() : canvasIndex( 0 ) {}
    uint32_t canvasIndex;
};

struct SegmentPath : public CanvasPath
{
    SegmentPath() : segmentIndex( 0 ) {}
    SegmentPath( const CanvasPath& p ) : CanvasPath( p ), segmentIndex( 0 ) {}
    uint32_t segmentIndex;
};

struct LayoutPath
{
    LayoutPath() : layoutIndex( 0 ) {}
    uint32_t layoutIndex;
};

struct ViewPath : public LayoutPath
{
    ViewPath() : viewIndex( 0 ) {}
    ViewPath( const LayoutPath& p ) : LayoutPath( p ), viewIndex( 0 ) {}
    uint32_t viewIndex;
};

}
}
#endif // EQSERVER_PATHS_H
