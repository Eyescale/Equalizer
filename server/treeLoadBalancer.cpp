
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "treeLoadBalancer.h"

#include "compound.h"
#include "log.h"

#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

#define MIN_PIXELS 8

std::ostream& operator << ( std::ostream& os, const TreeLoadBalancer::Node* );

// The tree load balancer organizes the children in a binary tree. At each
// level, a relative split position is determined by balancing the left subtree
// against the right subtree.

TreeLoadBalancer::TreeLoadBalancer( const LoadBalancer& parent )
        : LoadBalancerIF( parent ),
          _tree( 0 )
{
    EQINFO << "New TreeLoadBalancer @" << (void*)this << endl;
}

TreeLoadBalancer::~TreeLoadBalancer()
{
    _clearTree( _tree );
    delete _tree;
    _tree = 0;

    _history.clear();
}

void TreeLoadBalancer::update( const uint32_t frameNumber )
{
    if( _parent.isFrozen( ))
        return;

    if( !_tree )
    {
        const Compound*       compound = _parent.getCompound();
        const CompoundVector& children = compound->getChildren();
        if( children.empty( )) // leaf compound, can't do anything.
            return;

        _tree = _buildTree( children );
        EQINFO << "LB tree: " << _tree;
    }
    _checkHistory();

    // compute new data
    _history.push_back( LBFrameData( ));
    _history.back().first = frameNumber;

    _computeSplit();
}

TreeLoadBalancer::Node* TreeLoadBalancer::_buildTree( 
    const CompoundVector& compounds )
{
    Node*                    node     = new Node;
    const LoadBalancer::Mode mode     = _parent.getMode();

    if( compounds.size() == 1 )
    {
        Compound*                compound = compounds[0];

        node->compound  = compound;
        node->splitMode = ( mode == LoadBalancer::MODE_2D ) ? 
                              LoadBalancer::MODE_VERTICAL : mode;

        Channel* channel = compound->getChannel();
        EQASSERT( channel );
        channel->addListener( this );
        return node;
    }

    const size_t middle = compounds.size() / 2;

    CompoundVector left;
    for( size_t i = 0; i < middle; ++i )
        left.push_back( compounds[i] );

    CompoundVector right;
    for( size_t i = middle; i < compounds.size(); ++i )
        right.push_back( compounds[i] );

    node->left  = _buildTree( left );
    node->right = _buildTree( right );

    if( mode == LoadBalancer::MODE_2D )
        node->splitMode = 
            ( node->right->splitMode == LoadBalancer::MODE_VERTICAL ) ? 
                LoadBalancer::MODE_HORIZONTAL : LoadBalancer::MODE_VERTICAL;
    else
        node->splitMode = mode;
    node->time      = 0.0f;

    return node;
}

void TreeLoadBalancer::_clearTree( Node* node )
{
    if( !node )
        return;

    if( node->left )
        _clearTree( node->left );
    if( node->right )
        _clearTree( node->right );

    if( node->compound )
    {
        Channel* channel = node->compound->getChannel();
        EQASSERTINFO( channel, node->compound );
        channel->removeListener( this );
    }
}

void TreeLoadBalancer::notifyLoadData( Channel* channel, 
                                       const uint32_t frameNumber,
                                       const float startTime, 
                                       const float endTime
                                       /*, const float load */ )
{
    for( deque< LBFrameData >::iterator i = _history.begin();
         i != _history.end(); ++i )
    {
        LBFrameData& frameData = *i;
        if( frameData.first != frameNumber )
            continue;

        // Found corresponding historical data set
        LBDataVector& items = frameData.second;
        for( LBDataVector::iterator j = items.begin(); j != items.end(); ++j )
        {
            Data&     data     = *j;
            Compound* compound = data.compound;
            EQASSERT( compound );

            if( compound->getChannel() != channel )
                continue;

            // Found corresponding historical data item
            if( data.vp.getArea() <= 0.f )
                return;

            data.startTime = startTime;
            data.endTime   = endTime;
            data.load      = (endTime - startTime) / data.vp.getArea();
            EQLOG( LOG_LB ) << "Added load " << data.load << " (t=" 
                            << endTime-startTime << ") for " 
                            << channel->getName() << " " << data.vp << ", "
                            << data.range << " @ " << frameNumber << endl;
            return;

            // Note: if the same channel is used twice as a child, the 
            // load-compound association does not work.
        }
    }
}

void TreeLoadBalancer::_checkHistory()
{
    // 1. Find youngest complete load data set
    uint32_t useFrame = 0;
    for( std::deque< LBFrameData >::reverse_iterator i = _history.rbegin();
         i != _history.rend() && useFrame == 0; ++i )
    {
        const LBFrameData&  frameData  = *i;
        const LBDataVector& items      = frameData.second;
        bool                isComplete = true;

        for( LBDataVector::const_iterator j = items.begin();
             j != items.end() && isComplete; ++j )
        {
            const Data& data = *j;

            if( data.startTime < 0.f || data.endTime < 0.f )
                isComplete = false;
        }

        if( isComplete )
            useFrame = frameData.first;
    }

    // delete old, unneeded data sets
    while( !_history.empty() && _history.front().first < useFrame )
        _history.pop_front();
    
    if( _history.empty( )) // insert fake set
    {
        _history.resize( 1 );

        LBFrameData&  frameData  = _history.front();
        LBDataVector& items      = frameData.second;

        frameData.first = 0; // frameNumber
        items.resize( 1 );
        
        items[0].startTime = 0.f;
        items[0].endTime   = 1.f;
        items[0].load      = 1.f;
    }
}

void TreeLoadBalancer::_computeSplit()
{
    EQASSERT( !_history.empty( ));
    
    const LBFrameData&  frameData = _history.front();
    const LBDataVector& items     = frameData.second;

    // sort load items for each of the split directions
    LBDataVector sortedData[3] = { items, items, items };

    if( _parent.getMode() == LoadBalancer::MODE_DB )
    {
        LBDataVector& rangeData = sortedData[LoadBalancer::MODE_DB];
        sort( rangeData.begin(), rangeData.end(), _compareRange );
    }
    else
    {
        LBDataVector& xData = sortedData[LoadBalancer::MODE_VERTICAL];
        sort( xData.begin(), xData.end(), _compareX );

        for( LBDataVector::const_iterator i = xData.begin(); i != xData.end();
             ++i )
        {  
            const Data& data = *i;
            EQLOG( LOG_LB ) << data.vp << ", load " << data.load << " @ " <<
                frameData.first << endl;
        }

        LBDataVector& yData = sortedData[LoadBalancer::MODE_HORIZONTAL];
        sort( yData.begin(), yData.end(), _compareY );
    }


    // Compute total rendering time
    float totalTime = 0.0f;
    float endTime   = 0.0f;

    for( LBDataVector::const_iterator i = items.begin(); i != items.end(); ++i )
    {  
        const Data& data = *i;
        totalTime += ( data.endTime - data.startTime );
        endTime  = EQ_MAX( endTime, data.endTime );
    }

    const Compound* compound   = _parent.getCompound();
    const size_t    nResources = compound->getChildren().size();
    const float     timeLeft   = totalTime / nResources;
    EQLOG( LOG_LB ) << "Render time " << totalTime << ", per resource "
                    << timeLeft << endl;

    const float leftover = _assignTargetTimes( _tree, totalTime, timeLeft );
    EQASSERT( leftover <= totalTime * numeric_limits< float >::epsilon( ));
    if( leftover > totalTime * numeric_limits< float >::epsilon( ))
    {
	EQWARN << "Load balancer failure: " << leftover
	       << "ms not assigned for next frame" << endl;
    }

    _computeSplit( _tree, sortedData, eq::Viewport(), eq::Range() );
}

float TreeLoadBalancer::_assignTargetTimes( Node* node, const float totalTime, 
                                        const float resourceTime )
{
    if( node->compound )
    {
        float time = resourceTime; // default

#if 1 // disable to remove damping code
        float damping = _parent.getDamping();
        if( damping < 0.f )
            damping = .5f;
        EQASSERT( damping >= 0.f );
        EQASSERT( damping <= 1.f );

        const LBFrameData&  frameData = _history.front();
        const LBDataVector& items     = frameData.second;
        for( LBDataVector::const_iterator i = items.begin(); 
             i != items.end(); ++i )
        {
            const Data&     data     = *i;
            const Compound* compound = data.compound;

            if( compound != node->compound )
                continue;

            // found our last rendering time -> use this to smoothen the change:
            time = (1.f - damping) * resourceTime +
                   damping         * (data.endTime - data.startTime);
            break;
        }
#endif

        node->time = EQ_MIN( time, totalTime );
        EQLOG( LOG_LB ) << "Channel " << node->compound->getChannel()->getName()
                        << " target " << node->time << ", left " 
                        << totalTime - node->time << endl;

        return totalTime - node->time;
    }

    EQASSERT( node->left );
    EQASSERT( node->right );

    float timeLeft = _assignTargetTimes( node->left, totalTime, resourceTime );
    timeLeft       = _assignTargetTimes( node->right, timeLeft, resourceTime );
    node->time = node->left->time + node->right->time;
    
    EQLOG( LOG_LB ) << "Node time " << node->time << ", left " << timeLeft
                    << endl;
    return timeLeft;
}

void TreeLoadBalancer::_computeSplit( Node* node, LBDataVector* sortedData,
                                      const eq::Viewport& vp, 
                                      const eq::Range& range )
{
    const float time = node->time;
    EQLOG( LOG_LB ) << "_computeSplit " << vp << ", " << range << " time "
                    << time << endl;
    EQASSERTINFO( vp.isValid(), vp );
    EQASSERTINFO( range.isValid(), range );

    Compound* compound = node->compound;
    if( compound )
    {
        EQASSERTINFO( vp == eq::Viewport::FULL || range == eq::Range::ALL,
                      "Mixed 2D/DB load-balancing not implemented" );

        // TODO: check that time == vp * load
        compound->setViewport( vp );
        compound->setRange( range );

        EQLOG( LOG_LB ) << compound->getChannel()->getName() << " set "
                        << vp << ", " << range << endl;

        // save data for later use
        Data data;
        data.vp       = vp;
        data.range    = range;
        data.compound = compound;

        if( !vp.hasArea() || !range.isValid( )) // will not render
        {
            data.startTime = 0.f;
            data.endTime   = 0.f;
        } 

        LBFrameData&  frameData = _history.back();
        LBDataVector& items     = frameData.second;

        items.push_back( data );
        return;
    }

    EQASSERT( node->left && node->right );

    switch( node->splitMode )
    {
        case LoadBalancer::MODE_VERTICAL:
        {
            EQASSERT( range == eq::Range::ALL );

            float          timeLeft = node->left->time;
            float          splitPos = vp.x;
            LBDataVector workingSet = sortedData[LoadBalancer::MODE_VERTICAL];

            while( timeLeft > 0.f && !workingSet.empty( ))
            {
                EQLOG( LOG_LB ) << timeLeft << "ms left for "
                                << workingSet.size() << " tiles" << endl;

                // remove all irrelevant items from working set
                //   Is there a more clever way? Erasing invalidates iter, even
                //   if iter is copied and inc'd beforehand.
                LBDataVector newSet;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    if( data.vp.getXEnd() > splitPos )
                        newSet.push_back( data );
                }
                workingSet.swap( newSet );
                EQASSERT( !workingSet.empty( ));

                // find next 'discontinouity' in loads
                float currentPos = 1.0f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;                        
                    currentPos = EQ_MIN( currentPos, data.vp.getXEnd( ));
                }

                EQASSERTINFO( currentPos > splitPos,
                              currentPos << "<=" << splitPos );
                EQASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                EQLOG( LOG_LB ) << "Computing load in X " << splitPos << "..."
                                << currentPos << endl;
                float currentLoad = 0.f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                        
                    if( data.vp.x >= currentPos ) // not yet needed data sets
                        break;
#if 0
                    // make sure we cover full area
                    EQASSERTINFO(  data.vp.x <= splitPos, data.vp.x << " > "
                                   << splitPos );
                    EQASSERTINFO( data.vp.getXEnd() >= currentPos, 
                                  data.vp.getXEnd() << " < " << currentPos);
#endif
                    float       yContrib = data.vp.h;

                    if( data.vp.y < vp.y )
                        yContrib -= (vp.y - data.vp.y);
                    
                    const float dataEnd = data.vp.getYEnd();
                    const float vpEnd   = vp.getYEnd();
                    if( dataEnd > vpEnd )
                        yContrib -= (dataEnd - vpEnd);

                    if( yContrib > 0.f )
                    {
                        EQLOG( LOG_LB ) << data.vp << " contributes "
                                        << yContrib << " of " << data.vp.h
                                        << " with " << data.load << ": "
                                        << ( data.load * yContrib / vp.h )
                                        << " vp.y " << vp.y << " dataEnd " 
                                        << dataEnd << " vpEnd " << vpEnd
                                        << endl;

                        currentLoad += ( data.load * yContrib / vp.h );
                    }
                }

                const float width        = currentPos - splitPos;
                const float area         = width * vp.h;
                const float currentTime  = area * currentLoad;
                    
                EQLOG( LOG_LB ) << splitPos << "..." << currentPos 
                                << ": t=" << currentTime << " of " 
                                << timeLeft << endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    splitPos += (width * timeLeft / currentTime );
                    timeLeft = 0.0f;
                    EQASSERTINFO( splitPos <= vp.getXEnd(), 
                                  splitPos << " > " << vp.getXEnd( ));
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                    EQASSERTINFO( currentPos <= vp.getXEnd(), 
                                  currentPos << " > " << vp.getXEnd( ));
                }
            }
            EQASSERTINFO( timeLeft <= numeric_limits< float >::epsilon(), 
                          timeLeft );

            EQLOG( LOG_LB ) << "Split " << vp << " at X " << splitPos << endl;

            // Ensure minimum size
            const Compound* root    = _parent.getCompound();
            const float     epsilon = static_cast< float >( MIN_PIXELS ) /
                                      root->getInheritPixelViewport().w;

            if( (splitPos - vp.x) < epsilon )
                splitPos = vp.x + epsilon;
            if( (vp.getXEnd() - splitPos) < epsilon )
                splitPos = vp.getXEnd() - epsilon;

            splitPos = EQ_MAX( splitPos, vp.x );
            splitPos = EQ_MIN( splitPos, vp.getXEnd());

            // balance children
            eq::Viewport childVP = vp;
            childVP.w = (splitPos - vp.x);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.x = childVP.getXEnd();
            childVP.w = vp.getXEnd() - childVP.x;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case LoadBalancer::MODE_HORIZONTAL:
        {
            EQASSERT( range == eq::Range::ALL );
            float         timeLeft = node->left->time;
            float         splitPos = vp.y;
            LBDataVector  workingSet = sortedData[LoadBalancer::MODE_HORIZONTAL];

            while( timeLeft > 0.f && !workingSet.empty( ))
            {
                EQLOG( LOG_LB ) << timeLeft << "ms left for "
                                << workingSet.size() << " tiles" << endl;

                // remove all unrelevant items from working set
                //   Is there a more clever way? Erasing invalidates iter, even
                //   if iter is copied and inc'd beforehand.
                LBDataVector newSet;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    if( data.vp.getYEnd() > splitPos )
                        newSet.push_back( data );
                }
                workingSet.swap( newSet );
                EQASSERT( !workingSet.empty( ));

                // find next 'discontinouity' in loads
                float currentPos = 1.0f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;                        
                    currentPos = EQ_MIN( currentPos, data.vp.getYEnd( ));
                }

                EQASSERTINFO( currentPos > splitPos,
                              currentPos << "<=" << splitPos );
                EQASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                EQLOG( LOG_LB ) << "Computing load in Y " << splitPos << "..."
                                << currentPos << endl;
                float currentLoad = 0.f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                        
                    if( data.vp.y >= currentPos ) // not yet needed data sets
                        break;
#if 0
                    // make sure we cover full area
                    EQASSERTINFO(  data.vp.y <= splitPos, data.vp.y << " > "
                                   << splitPos );
                    EQASSERTINFO( data.vp.getYEnd() >= currentPos, 
                                  data.vp.getYEnd() << " < " << currentPos);
#endif
                    float       xContrib = data.vp.w;

                    if( data.vp.x < vp.x )
                        xContrib -= (vp.x - data.vp.x);
                    
                    const float dataEnd = data.vp.getXEnd();
                    const float vpEnd   = vp.getXEnd();
                    if( dataEnd > vpEnd )
                        xContrib -= (dataEnd - vpEnd);
                    
                    if( xContrib > 0.f )
                    {
                        EQLOG( LOG_LB ) << data.vp << " contributes "
                                        << xContrib << " of " << data.vp.w
                                        << " with " << data.load << ": "
                                        << ( data.load * xContrib / vp.w )
                                        << " vp.x " << vp.x << " dataEnd " 
                                        << dataEnd << " vpEnd " << vpEnd
                                        << endl;

                        currentLoad += ( data.load * xContrib / vp.w );
                    }
                }

                const float height       = currentPos - splitPos;
                const float area         = height * vp.w;
                const float currentTime  = area * currentLoad;
                    
                EQLOG( LOG_LB ) << splitPos << "..." << currentPos 
                                << ": t=" << currentTime << " of " 
                                << timeLeft << endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    splitPos += (height * timeLeft / currentTime );
                    timeLeft = 0.0f;
                    EQASSERTINFO( splitPos <= vp.getYEnd(), 
                                  splitPos << " > " << vp.getYEnd( ));
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                    EQASSERTINFO( currentPos <= vp.getYEnd(), 
                                  currentPos << " > " << vp.getYEnd( ));
                }
            }
            EQASSERTINFO( timeLeft <= numeric_limits< float >::epsilon(), 
                          timeLeft );

            EQLOG( LOG_LB ) << "Split " << vp << " at Y " << splitPos << endl;

            const Compound* root    = _parent.getCompound();
            const float     epsilon = static_cast< float >( MIN_PIXELS ) /
                                      root->getInheritPixelViewport().h;

            if( (splitPos - vp.y) < epsilon )
                splitPos = vp.y + epsilon;
            if( (vp.getYEnd() - splitPos) < epsilon )
                splitPos = vp.getYEnd() - epsilon;

            splitPos = EQ_MAX( splitPos, vp.y );
            splitPos = EQ_MIN( splitPos, vp.getYEnd());

            eq::Viewport childVP = vp;
            childVP.h = (splitPos - vp.y);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.y = childVP.getYEnd();
            childVP.h = vp.getYEnd() - childVP.y;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case LoadBalancer::MODE_DB:
        {
            EQASSERT( vp == eq::Viewport::FULL );
            float          timeLeft = node->left->time;
            float          splitPos = range.start;
            LBDataVector workingSet = sortedData[LoadBalancer::MODE_DB];

            while( timeLeft > 0.f && !workingSet.empty( ))
            {
                EQLOG( LOG_LB ) << timeLeft << "ms left for "
                                << workingSet.size() << " tiles" << endl;

                // remove all irrelevant items from working set
                //   Is there a more clever way? Erasing invalidates iter, even
                //   if iter is copied and inc'd beforehand.
                LBDataVector newSet;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    if( data.range.end > splitPos )
                        newSet.push_back( data );
                }
                workingSet.swap( newSet );
                EQASSERT( !workingSet.empty( ));

                // find next 'discontinouity' in loads
                float currentPos = 1.0f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;                        
                    currentPos = EQ_MIN( currentPos, data.range.end );
                }

                EQASSERTINFO( currentPos > splitPos,
                              currentPos << "<=" << splitPos );
                EQASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                EQLOG( LOG_LB ) << "Computing load in range " << splitPos
                                << "..." << currentPos << endl;
                float currentLoad = 0.f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                        
                    if( data.range.start >= currentPos ) // not yet needed data
                        break;
#if 0
                    // make sure we cover full area
                    EQASSERTINFO(  data.range.start <= splitPos, 
                                   data.range.start << " > " << splitPos );
                    EQASSERTINFO( data.range.end >= currentPos, 
                                  data.range.end << " < " << currentPos);
#endif
                    currentLoad += data.load;
                }

                EQLOG( LOG_LB ) << splitPos << "..." << currentPos 
                                << ": t=" << currentLoad << " of " 
                                << timeLeft << endl;

                if( currentLoad >= timeLeft ) // found last region
                {
                    const float width = currentPos - splitPos;
                    splitPos += (width * timeLeft / currentLoad );
                    timeLeft = 0.0f;
                    EQASSERTINFO( splitPos <= range.end, 
                                  splitPos << " > " << range.end );
                }
                else
                {
                    timeLeft -= currentLoad;
                    splitPos  = currentPos;
                    EQASSERTINFO( currentPos <= range.end, 
                                  currentPos << " > " << range.end );
                }
            }
            EQASSERTINFO( timeLeft <= numeric_limits< float >::epsilon(), 
                          timeLeft );

            EQLOG( LOG_LB ) << "Split " << range << " at pos " << splitPos
                            << endl;


            eq::Range childRange = range;
            childRange.end       = splitPos;
            _computeSplit( node->left, sortedData, vp, childRange );

            childRange.start = childRange.end;
            childRange.end   = range.end;
            _computeSplit( node->right, sortedData, vp, childRange );
            break;
        }

        default:
            EQUNIMPLEMENTED;
    }
}

ostream& operator << ( ostream& os, const TreeLoadBalancer::Node* node )
{
    if( !node )
        return os;

    os << disableFlush;

    if( node->compound )
        os << node->compound->getChannel()->getName() << " target time " 
           << node->time << endl;
    else
        os << "split " << node->splitMode << " target time " << node->time
           << endl << indent << node->left << node->right << exdent;

    os << enableFlush;
    return os;
}

}
}
