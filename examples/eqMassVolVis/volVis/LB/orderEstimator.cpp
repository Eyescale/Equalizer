
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#include "../asyncFetcher/gpuCacheManager.h"

#include "orderEstimator.h"
#include "cameraParameters.h"

#include <msv/tree/volumeTreeBase.h>

#include <list>
#include <algorithm>


namespace massVolVis
{


OrderEstimator::OrderEstimator()
    : _cpPtr( new CameraParameters() )
    , _timePerBlockUnitSize( 10.0 / 500.0 )
{
}


struct OctreeChildrenOrder
{
    OctreeChildrenOrder() { reset(); }
    byte pos[8];

    void reset()
    {
        for( int i = 0; i < 8; ++i )
            pos[i] = i;
    }
};


namespace
{
void _getChildrenOrder( OctreeChildrenOrder& order, const Box_f& box, const CameraParameters& cp )
{
    const Vec3_f c = box.getCenter();
    const Vector4f center( c.x, c.y, c.z, 1.0 );
    bool xNS, yNS, zNS;

    cp.normalsProductsSigns( center, xNS, yNS, zNS );

    order.reset();

    if( !xNS )
    {
        std::swap( order.pos[0], order.pos[1] );
        std::swap( order.pos[2], order.pos[3] );
        std::swap( order.pos[4], order.pos[5] );
        std::swap( order.pos[6], order.pos[7] );
    }
    if( !yNS )
    {
        std::swap( order.pos[0], order.pos[2] );
        std::swap( order.pos[1], order.pos[3] );
        std::swap( order.pos[4], order.pos[6] );
        std::swap( order.pos[5], order.pos[7] );
    }
    if( !zNS )
    {
        std::swap( order.pos[0], order.pos[4] );
        std::swap( order.pos[1], order.pos[5] );
        std::swap( order.pos[2], order.pos[6] );
        std::swap( order.pos[3], order.pos[7] );
    }
}

/** 
 * Returns coordinates of the block in float coordinates mapped to [-1..1] range.
 */
Box_f _getFloatDataPos( const VolumeTreeBase& tree )
{
    const float   s  = tree.getMaxBBSize();
    const Box_i32 ic = tree.getAbsCoordinates( 0 );
//    const Box_i32 ic = tree.getDataCoordinates();
    return Box_f( Vec3_f( ic.s.x/s, ic.s.y/s, ic.s.z/s ),
                  Vec3_f( ic.e.x/s, ic.e.y/s, ic.e.z/s ))-0.5f;
}

Box_f _getFloatDataPos2( const VolumeTreeBase& tree )
{
    const float   s  = tree.getMaxBBSize();
//    const Box_i32 ic = tree.getAbsCoordinates( 0 );
    const Box_i32 ic = tree.getDataCoordinates();
    return Box_f( Vec3_f( ic.s.x/s, ic.s.y/s, ic.s.z/s ),
                  Vec3_f( ic.e.x/s, ic.e.y/s, ic.e.z/s ))-0.5f;
}
}

// TODO: compute rendering time properly by computing projected size and statistics from previous frames
float OrderEstimator::_estimateRenderingTime( const uint32_t size )
{
    return _timePerBlockUnitSize * size;
}

void OrderEstimator::updateRenderingStats( const RenderNodeVec& renderNodes, double totalTimeMs )
{
    if( renderNodes.size() == 0 )
        return;

    uint32_t totalSize = 0;
    for( size_t i = 0; i < renderNodes.size(); ++i )
        totalSize += renderNodes[i].screenSize;

    totalTimeMs /= static_cast<double>(totalSize); // time per block
    const double a = 0.9;
    _timePerBlockUnitSize = _timePerBlockUnitSize*a + totalTimeMs*(1.0-a);

//    static int i = 0;
//    if( (++i) % 100 == 0 )
//        std::cout << "time per block: " << _timePerBlockUnitSize << std::endl;
}


typedef std::list<RenderNode>::iterator RenderNodeListIt;


bool RenderNodeListItCmp( const RenderNodeListIt& first, const RenderNodeListIt& second )
{
    return (*first).screenRectSquare < (*second).screenRectSquare;
}

namespace
{

bool _validResolution( bool screenSizeValid, bool useRenderingError, uint16_t renderingError, int32_t pos )
{
    return screenSizeValid;
}
}

// sorted list - rendering queue order
// sorted list of refferences according to the screen size
//  - analyze only second one
//  - remove skip field from the Struct
void OrderEstimator::compute(
            RenderNodeVec&    renderNodes,
            NodeIdPosVec&     desiredIds,
     const  GPUCacheManager&  gpuManager,
     const  VolumeTreeBase&   tree,
     const  Box_f&            bb,
     const  CameraParameters& cp,
     const  uint32_t          nodesMax,
     const  uint32_t          tfHist,
     const  float             msMax,
     const  uint8_t           maxTreeDepth,
     const  bool              frontToBack,
            bool              useRenderingError,
     const  uint16_t          renderingError  )
{
    renderNodes.resize( 0 );
    desiredIds.resize(  0 );

    // Check if buget is allocated
    if( nodesMax < 1 )
    {
        LBWARN << "budget is zero" << std::endl;
        return;
    }

    // Check if tree is initialized
    const uint32_t rootNodePos = 0;
    const NodeId   rootNodeId = tree.getNodeId( rootNodePos );
    if( tree.getSize() < 1 || rootNodeId < 1 )
    {
        LBWARN << "octree is not initialized" << std::endl;
        return;
    }

    useRenderingError = false;

    // Check if root node is loaded
    if( !gpuManager.hasNodeOnGPU( rootNodeId ))
    {
        // always keep root node loaded
        desiredIds.push_back( NodeIdPos( rootNodeId, rootNodePos ));
        return;
    }

    // Initialize rendering queue with the root node and
    // then try to expand it
    RenderNode rNode;
    rNode.nodeId        = rootNodeId;
    rNode.coords        = _getFloatDataPos( tree );
    rNode.treePos       = rootNodePos;
    rNode.treeLevel     = 1;
    rNode.screenRect    = cp.computeScreenProjectionRect( rNode.coords );
    rNode.screenRectSquare = rNode.screenRect.getAreaSize();
    rNode.screenSize    = rNode.screenRect.getDiagonalSize();
    rNode.renderingTime = _estimateRenderingTime( rNode.screenSize );
    rNode.fullyVsible   = cp.computeVisibility( rNode.coords ) == vmml::VISIBILITY_FULL;

    std::list<RenderNode> renderList;
    std::vector<RenderNodeListIt> renderHeap;

    renderList.push_front( rNode );
    renderHeap.push_back( renderList.begin() );

    std::make_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp );

    uint32_t nodesUsed = 1;
    float    timeUsed  = rNode.renderingTime;

    // blocks to render stay in renderList, if expansion is desired but
    // not yet possible (data is not on GPU) corresponding requests are 
    // copied to desiredIds array.
    RenderNode childrenNodes[8];
    Box_f      quads[8];
    OctreeChildrenOrder childrenOrder;
    bool expanded = true;

    while( expanded && nodesUsed+8 < nodesMax && timeUsed < msMax )
    {
        expanded = false;
        while( renderHeap.size() > 0 )
        {
            const RenderNode* current = &(*renderHeap.front());

            if( current->treeLevel >= maxTreeDepth )
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            if( !tree.hasVisibleData( current->treePos, tfHist ))
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            // if leaf node - don't expand, skip in the future
            if( !tree.hasChildren( current->treePos ))
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            // Check that resolution is sufficient
//            LBWARN << "nS: " << current->screenSize << " tS: " << tree.getBlockSize() << std::endl;
            bool screenSizeValid = static_cast<int>( current->screenSize )/2.8 < tree.getBlockSize();
//            if( screenSizeValid )
            if( _validResolution( screenSizeValid, useRenderingError, renderingError, current->treePos ))
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            // node is expandable:
            //  - check how much it costs to expand (only withing view frustum), proceed if enougth resources:
            //    - if all children that are in the view frustum are also on GPU, then replace root (put root to desired)
            _getChildrenOrder( childrenOrder, current->coords, cp );

            current->coords.getQuadrants( quads );

            const int64_t childrenPos = tree.getChild( current->treePos );

            uint32_t count    = 0; // visible child nodes
            uint32_t countGPU = 0; // visible child nodes on GPU
            for( int i = 0; i < 8; ++i )
            {
                const uint32_t childPos = childrenPos + childrenOrder.pos[i];
                const uint32_t childId  = tree.getNodeId( childPos );
                if( childId == 0 ) // empty node
                    continue;

                if( !tree.hasVisibleData( childPos, tfHist ))
                    continue;

                RenderNode& node = childrenNodes[ count ];

                node.nodeId     = childId;
                node.coords     = quads[ childrenOrder.pos[i] ];
                node.treePos    = childPos;

                // check if child fits to required data BB
                if( !bb.intersect( node.coords ).valid() )
                    continue;

                // check if child is visible
                if( current->fullyVsible )
                {
                    node.fullyVsible = true;
                }else
                {
                    const vmml::Visibility visibility = cp.computeVisibility( node.coords );
                    if( visibility == vmml::VISIBILITY_NONE )
                        continue;

                    node.fullyVsible = (visibility == vmml::VISIBILITY_FULL);
                }

                // check if child is on GPU
                if( gpuManager.hasNodeOnGPU( node.nodeId ))
                    countGPU++;
                count++;
            }
            if( count == 0 ) // no valid children
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }
            if( nodesUsed + count - 1 > nodesMax ) // too many nodes already
            {
                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            float childrenRenderingTime = 0;
            if( count == countGPU )// everything is on GPU, compute time
            {
                for( uint32_t i = 0; i < count; ++i )
                {
                    RenderNode& node = childrenNodes[ i ];

                    node.screenRect    = cp.computeScreenProjectionRect( node.coords );
                    node.screenRectSquare = node.screenRect.getAreaSize();
                    node.screenSize    = node.screenRect.getDiagonalSize();
                    node.renderingTime = _estimateRenderingTime( node.screenSize );

                    EQASSERT( current->treeLevel > 0 );
                    node.treeLevel  = current->treeLevel + 1;

                    childrenRenderingTime += node.renderingTime;

                    if( timeUsed - current->renderingTime + childrenRenderingTime > msMax )
                        break;
                }
            }

            // we will add all children to loading or rendering queue now
            nodesUsed += count; // adjust budget

            bool childrenQualityTooHigh = useRenderingError;

            if( count != countGPU || // some children are not on GPU or rendering of children would take too long
                timeUsed - current->renderingTime + childrenRenderingTime > msMax ||
                childrenQualityTooHigh )
            {
                // scedule children for loading
                for( uint32_t i = 0; i < count; ++i )
                    desiredIds.push_back(
                        NodeIdPos( childrenNodes[i].nodeId, childrenNodes[i].treePos ));

                pop_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp ); renderHeap.pop_back();
                continue;
            }

            // will render children instead of the root
            // adjust used time
            timeUsed = timeUsed - current->renderingTime + childrenRenderingTime;

            // replace root with children
            desiredIds.push_back( NodeIdPos( current->nodeId, current->treePos ));

            // replace root with first child
            RenderNodeListIt parentId = renderHeap.front();
            pop_heap(  renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp );
            *parentId = childrenNodes[ 0 ];
            push_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp );
            ++parentId;

            // insert other children
            for( uint32_t i = 1; i < count; ++i )
            {
                RenderNodeListIt childId = renderList.insert( parentId, childrenNodes[ i ] );
                renderHeap.push_back( childId );
                push_heap( renderHeap.begin(), renderHeap.end(), RenderNodeListItCmp );
            }

            expanded = true;
            if( timeUsed >= msMax || nodesUsed+8 >= nodesMax )
                break;
        }
    }

    // now renderList has desired set of blocks, we copy rendering front to
    // the destination storage "renderNodes", additionally node are Ids duplicated to
    // desiredIds array to keep them on GPU
    if( frontToBack )
    {
        for( RenderNodeListIt it = renderList.begin(); it != renderList.end(); ++it )
        {
            RenderNode* current = &(*it);
            renderNodes.push_back( *current );
            desiredIds.push_back( NodeIdPos( current->nodeId, current->treePos ));
        }
    }else
    {
        RenderNodeListIt it = renderList.end();
        while( it != renderList.begin() )
        {
            --it;
            RenderNode* current = &(*it);
            renderNodes.push_back( *current );
            desiredIds.push_back( NodeIdPos( current->nodeId, current->treePos ));
        }
    }
}


namespace
{
void _makeDataSplit( vecBoxN_f& bbs, const Box_f& bb,
                     const int32_t nS, const int32_t nE,
                     const CameraParameters& cp )
{
    if( nE < nS )
        return;
    if( nE == nS )
    {
        bbs.push_back( BoxN_f( bb, nS ));
        return;
    }

    const uint32_t ln = ( nE-nS+2 ) / 2;
    const uint32_t mn = nS + ln - 1;
    const float    lr = ln / float( nE-nS+1 );

    // Split data box along the longest axis, according
    // to the number of nodes
    const Vec3_f dim = bb.getDim();
    const Vec3_f bC  = bb.getCenter();
//      LBWARN << dim << std::endl;
    Box_f lBox;
    Box_f rBox;
    bool  invert;
    if( dim.x >= dim.y && dim.x >= dim.z )
    {
        const float midX = bb.s.x + lr * dim.w;
        lBox = Box_f( bb.s, Vec3_f( midX, bb.e.y, bb.e.z ));
        rBox = Box_f( Vec3_f( midX, bb.s.y, bb.s.z ), bb.e );
        invert = cp.xNormalProductSign( Vector4f( midX, bC.y, bC.z, 1.0 ));
    }else
    if( dim.y >= dim.x && dim.y >= dim.z )
    {
        const float midY = bb.s.y + lr * dim.h;
        lBox = Box_f( bb.s, Vec3_f( bb.e.x, midY, bb.e.z ));
        rBox = Box_f( Vec3_f( bb.s.x, midY, bb.s.z ), bb.e );
        invert = cp.yNormalProductSign( Vector4f( bC.x, midY, bC.z, 1.0 ));
    }else
    {
        LBASSERT( dim.z >= dim.x && dim.z >= dim.y );
        const float midZ = bb.s.z + lr * dim.d;
        lBox = Box_f( bb.s, Vec3_f( bb.e.x, bb.e.y, midZ ));
        rBox = Box_f( Vec3_f( bb.s.x, bb.s.y, midZ ), bb.e );
        invert = cp.zNormalProductSign( Vector4f( bC.x, bC.y, midZ, 1.0 ));
    }
    if( invert )
    {
        _makeDataSplit( bbs, lBox, nS,   mn, cp );
        _makeDataSplit( bbs, rBox, mn+1, nE, cp );
    }else
    {
        _makeDataSplit( bbs, rBox, mn+1, nE, cp );
        _makeDataSplit( bbs, lBox, nS,   mn, cp );
    }
}
}


const vecBoxN_f& OrderEstimator::getDataSplit( const uint32_t          n,
                                               const VolumeTreeBase&   tree,
                                               const CameraParameters& cp )
{
    const Box_f dBB = _getFloatDataPos2( tree );
    if( _bbs.size() == n && dBB == _dataBB && cp == *_cpPtr )
        return _bbs;
    // else compute new set of data boxes

    _dataBB = dBB;
    *_cpPtr    = cp;
    _bbs.clear();

    _makeDataSplit( _bbs, _dataBB, 0, n-1, cp );
    LBASSERT( _bbs.size() == n );
    return _bbs;
}

} //namespace massVolVis

