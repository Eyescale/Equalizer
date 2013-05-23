/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__RENDER_NODE_H
#define MASS_VOL__RENDER_NODE_H

#include <msv/tree/nodeId.h>
#include <msv/types/box.h>
#include <msv/types/rectangle.h>

namespace massVolVis
{

struct RenderNode
{
    RenderNode()
        : nodeId( 0 )
        , treeLevel( 0 )
        , treePos( 0 )
        , screenRectSquare( 0 )
        , screenSize( 0 )
        , renderingTime( 0 )
        , fullyVsible( false ){}

    NodeId   nodeId;
    Box_f    coords;
    byte     treeLevel;

    uint32_t treePos;      // position in the octree
    Rect_i32 screenRect;
    uint32_t screenRectSquare;
    uint32_t screenSize;
    float    renderingTime;
    bool     fullyVsible;
};

typedef std::vector<RenderNode> RenderNodeVec;

namespace
{
std::ostream& operator << ( std::ostream& os, const RenderNode& rn )
{
    os << "nId: " << rn.nodeId << " coords: " << rn.coords; 
    return os;
}
}

}

#endif //MASS_VOL__RENDER_NODE_H
