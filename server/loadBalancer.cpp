
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "loadBalancer.h"

#include "compound.h"
#include "log.h"

#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

#define MIN_PIXELS 1

std::ostream& operator << ( std::ostream& os, const LoadBalancer::Node* node );

// The load balancer organizes the children in a binary tree. At each level, a
// relative split position is determined by balancing the left subtree against
// the right subtree.

LoadBalancer::LoadBalancer()
        : _mode( MODE_2D )
        , _compound( 0 )
        , _tree( 0 )
        , _freeze( false )
{
    EQINFO << "New LoadBalancer @" << (void*)this << endl;
}

LoadBalancer::LoadBalancer( const LoadBalancer& from )
        : CompoundListener()
        , ChannelListener()
        , _mode( from._mode )
        , _compound( 0 )
        , _tree( 0 )
{
}

LoadBalancer::~LoadBalancer()
{
    attach( 0 );
    _clear();
}

void LoadBalancer::attach( Compound* compound )
{
    if( _compound )
    {
        _compound->removeListener( this );
        _compound = 0;
    }

    _clear();

    if( compound )
    {
        _compound = compound;
        compound->addListener( this );
    }
}

void LoadBalancer::_clear()
{
    _clearTree( _tree );
    delete _tree;
    _tree = 0;

    _history.clear();
}

void LoadBalancer::notifyChildAdded( Compound* compound, Compound* child )
{
    _clear();
}

void LoadBalancer::notifyChildRemove( Compound* compound, Compound* child )
{
    _clear();
}

void LoadBalancer::notifyUpdatePre( Compound* compound,
                                    const uint32_t frameNumber )
{
    if( _freeze )
        return;

    EQASSERT( _compound );
    EQASSERT( _compound == compound );
    
    if( !_tree )
    {
        const CompoundVector& children = _compound->getChildren();
        if( children.empty( )) // leaf compound, can't do anything.
            return;

        _tree = _buildTree( children );
        EQINFO << "LB tree: " << _tree;
    }
    EQLOG( LOG_LB ) << "Balance for frame " << frameNumber << endl;

    _checkHistory();

    // compute new data
    _history.push_back( LBFrameData( ));
    _history.back().first = frameNumber;

    _computeSplit();
}

LoadBalancer::Node* LoadBalancer::_buildTree( const CompoundVector& compounds )
{
    if( compounds.size() == 1 )
    {
        Node* node = new Node;

        Compound* compound = compounds[0];
        node->compound  = compound;
        node->splitMode = ( _mode == MODE_2D ) ? MODE_VERTICAL : _mode;

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


    Node* node = new Node;
    node->left  = _buildTree( left );
    node->right = _buildTree( right );

    if( _mode == MODE_2D )
        node->splitMode = ( node->right->splitMode == MODE_VERTICAL ) ? 
                                MODE_HORIZONTAL : MODE_VERTICAL;
    else
        node->splitMode = _mode;
    node->time      = 0.0f;

    return node;
}

void LoadBalancer::_clearTree( Node* node )
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
        EQASSERT( channel );
        channel->removeListener( this );
    }
}

void LoadBalancer::notifyLoadData( Channel* channel, const uint32_t frameNumber,
                                   const float startTime, const float endTime
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

void LoadBalancer::_checkHistory()
{
    // 1. Find youngest complete load data set
    uint32_t useFrame = 0;
    for( std::deque< LBFrameData >::reverse_iterator i =_history.rbegin();
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

void LoadBalancer::_computeSplit()
{
    EQASSERT( !_history.empty( ));
    
    const LBFrameData&  frameData = _history.front();
    const LBDataVector& items     = frameData.second;

    // sort load items for each of the split directions
    LBDataVector sortedData[3] = { items, items, items };

    if( _mode == MODE_DB )
    {
        LBDataVector& rangeData = sortedData[MODE_DB];
        sort( rangeData.begin(), rangeData.end(), _compareRange );
    }
    else
    {
        LBDataVector& xData = sortedData[MODE_VERTICAL];
        sort( xData.begin(), xData.end(), _compareX );

        for( LBDataVector::const_iterator i = xData.begin(); i != xData.end();
             ++i )
        {  
            const Data& data = *i;
            EQLOG( LOG_LB ) << data.vp << ", load " << data.load << " @ " <<
                frameData.first << endl;
        }

        LBDataVector& yData = sortedData[MODE_HORIZONTAL];
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

    const size_t nResources = _compound->getChildren().size();
    const float maxIdleTime = 0.f; //.1f * totalTime;
    EQLOG( LOG_LB ) << "Render time " << totalTime << ", max idle "
                    << maxIdleTime << endl;

    const float    idleTime = _assignIdleTimes( _tree, items, maxIdleTime,
                                                endTime );
    const float    timeLeft = EQ_MAX( 0.0f, totalTime - idleTime ) / nResources;

    EQLOG( LOG_LB ) << "Idle time " << idleTime << ", left per resource "
                    << timeLeft << endl;

    const float leftover = _assignTargetTimes( _tree, totalTime, timeLeft );
    EQASSERT( leftover <= totalTime * numeric_limits< float >::epsilon( ));

    _computeSplit( _tree, sortedData, eq::Viewport(), eq::Range() );
}

float LoadBalancer::_assignIdleTimes( Node* node, const LBDataVector& items,
                                      const float maxIdleTime, 
                                      const float endTime )
{
    if( node->left )
    {
        EQASSERT( node->right );

        return _assignIdleTimes( node->left,  items, maxIdleTime, endTime ) + 
               _assignIdleTimes( node->right, items, maxIdleTime, endTime );
    }

    const Compound* compound = node->compound;
    EQASSERT( compound );
    
    const Channel* channel = compound->getChannel();
    EQASSERT( channel );

    for( LBDataVector::const_iterator i = items.begin(); i != items.end(); ++i )
    {
        const Data& data = *i;
        if( data.compound && data.compound->getChannel() == channel )
        {
            node->time = EQ_MIN( maxIdleTime, endTime - data.endTime );

            EQLOG( LOG_LB ) << "Channel " << channel->getName() << " early " 
                            << node->time << endl;
            return node->time;
        }
    }

    node->time = 0.0f;
    return 0.0f;
}

float LoadBalancer::_assignTargetTimes( Node* node, const float totalTime, 
                                        const float resourceTime )
{
    if( node->compound )
    {
        node->time = EQ_MIN( node->time + resourceTime, totalTime );
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

void LoadBalancer::_computeSplit( Node* node, LBDataVector* sortedData,
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
        case MODE_VERTICAL:
        {
            EQASSERT( range == eq::Range::ALL );

            float          timeLeft = node->left->time;
            float          splitPos = vp.x;
            LBDataVector workingSet = sortedData[MODE_VERTICAL];

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

            const float epsilon = static_cast< float >( MIN_PIXELS ) /
                                  _compound->getInheritPixelViewport().w;

            if( (splitPos - vp.x) < epsilon )
                splitPos = vp.x;
            if( (vp.getXEnd() - splitPos) < epsilon )
                splitPos = vp.getXEnd();

            eq::Viewport childVP = vp;
            childVP.w = (splitPos - vp.x);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.x = childVP.getXEnd();
            childVP.w = vp.getXEnd() - childVP.x;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case MODE_HORIZONTAL:
        {
            EQASSERT( range == eq::Range::ALL );
            float         timeLeft = node->left->time;
            float         splitPos = vp.y;
            LBDataVector  workingSet = sortedData[MODE_HORIZONTAL];

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

            const float epsilon = static_cast< float >( MIN_PIXELS ) /
                                  _compound->getInheritPixelViewport().h;

            if( (splitPos - vp.y) < epsilon )
                splitPos = vp.y;
            if( (vp.getYEnd() - splitPos) < epsilon )
                splitPos = vp.getYEnd();

            eq::Viewport childVP = vp;
            childVP.h = (splitPos - vp.y);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.y = childVP.getYEnd();
            childVP.h = vp.getYEnd() - childVP.y;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case MODE_DB:
        {
            EQASSERT( vp == eq::Viewport::FULL );
            float          timeLeft = node->left->time;
            float          splitPos = range.start;
            LBDataVector workingSet = sortedData[MODE_DB];

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

std::ostream& operator << ( std::ostream& os, 
                            const LoadBalancer::Mode mode )
{
    os << ( mode == LoadBalancer::MODE_2D         ? "2D" :
            mode == LoadBalancer::MODE_VERTICAL   ? "VERTICAL" :
            mode == LoadBalancer::MODE_HORIZONTAL ? "HORIZONTAL" :
            mode == LoadBalancer::MODE_DB         ? "DB" : "ERROR" );
    return os;
}

std::ostream& operator << ( std::ostream& os, const LoadBalancer* lb )
{
    os << disableFlush
       << "loadBalancer " << endl
       << '{' << endl
       << "    mode " << lb->getMode() << endl
       << '}' << endl << enableFlush;
    return os;
}

std::ostream& operator << ( std::ostream& os, const LoadBalancer::Node* node )
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
