

/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "loadEqualizer.h"

#include "../compound.h"
#include "../log.h"

#include <eq/client/client.h>
#include <eq/client/server.h>
#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

#define MIN_PIXELS 8

std::ostream& operator << ( std::ostream& os, const LoadEqualizer::Node* );

// The tree load balancer organizes the children in a binary tree. At each
// level, a relative split position is determined by balancing the left subtree
// against the right subtree.

LoadEqualizer::LoadEqualizer()
        : _mode( MODE_2D )
        , _damping( .5f )
        , _tree( 0 )
{
    EQINFO << "New LoadEqualizer @" << (void*)this << endl;
}

LoadEqualizer::LoadEqualizer( const LoadEqualizer& from )
        : Equalizer( from )
        , ChannelListener( from )
        , _mode( from._mode )
        , _damping( from._damping )
        , _tree( 0 )
{}

LoadEqualizer::~LoadEqualizer()
{
    _clearTree( _tree );
    delete _tree;
    _tree = 0;

    _history.clear();
}

void LoadEqualizer::notifyUpdatePre( Compound* compound,
                                     const uint32_t frameNumber )
{
    if( isFrozen( ))
        return;

    if( !_tree )
    {
        EQASSERT( compound == getCompound( ));
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

LoadEqualizer::Node* LoadEqualizer::_buildTree( const CompoundVector& compounds)
{
    Node* node = new Node;

    if( compounds.size() == 1 )
    {
        Compound*                compound = compounds[0];

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

void LoadEqualizer::_clearTree( Node* node )
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

void LoadEqualizer::notifyLoadData( Channel* channel,
                                    const uint32_t frameNumber,
                                    const uint32_t nStatistics,
                                    const eq::Statistic* statistics )
{
    // gather relevant load data
    float startTime = numeric_limits< float >::max();
    float endTime   = 0.0f;
    bool  loadSet   = false;
    float transmitTime = 0.0f;

    for( uint32_t i = 0; i < nStatistics && !loadSet; ++i )
    {
        const eq::Statistic& data = statistics[i];
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_TRANSMIT:
#ifdef EQ_ASYNC_TRANSMIT
                transmitTime = data.endTime - data.startTime;
                break;
#else
                // no break;
#endif
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
                //case eq::Statistic::CHANNEL_DRAW_FINISH:
            case eq::Statistic::CHANNEL_READBACK:
                startTime = EQ_MIN( startTime, data.startTime );
                endTime   = EQ_MAX( endTime, data.endTime );
                break;
                
                // assemble blocks on frames, stop using subsequent data
            case eq::Statistic::CHANNEL_ASSEMBLE:
                loadSet = true;
                break;
                
            default:
                break;
        }
    }
    
    if( startTime == numeric_limits< float >::max( ))
        return;
    
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

            data.time = endTime - startTime;
            data.time = EQ_MAX( data.time, transmitTime );
            data.load = data.time / data.vp.getArea();
            EQLOG( LOG_LB ) << "Added load " << data.load << " (t=" << data.time
                            << ") for " << channel->getName() << " " << data.vp
                            << ", " << data.range << " @ " << frameNumber
                            << std::endl;
            return;

            // Note: if the same channel is used twice as a child, the 
            // load-compound association does not work.
        }
    }
}

void LoadEqualizer::_checkHistory()
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

            if( data.time < 0.f )
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
        
        items[0].time = 1.f;
        items[0].load = 1.f;
    }
}

void LoadEqualizer::_computeSplit()
{
    EQASSERT( !_history.empty( ));
    
    const LBFrameData&  frameData = _history.front();
    const LBDataVector& items     = frameData.second;

    // sort load items for each of the split directions
    LBDataVector sortedData[3] = { items, items, items };

    if( _mode == MODE_DB )
    {
        LBDataVector& rangeData = sortedData[ MODE_DB ];
        sort( rangeData.begin(), rangeData.end(), _compareRange );
    }
    else
    {
        LBDataVector& xData = sortedData[ MODE_VERTICAL ];
        sort( xData.begin(), xData.end(), _compareX );

        for( LBDataVector::const_iterator i = xData.begin(); i != xData.end();
             ++i )
        {  
            const Data& data = *i;
            EQLOG( LOG_LB ) << data.vp << ", load " << data.load << " @ " <<
                frameData.first << endl;
        }

        LBDataVector& yData = sortedData[ MODE_HORIZONTAL ];
        sort( yData.begin(), yData.end(), _compareY );
    }


    // Compute total rendering time
    float totalTime = 0.0f;

    for( LBDataVector::const_iterator i = items.begin(); i != items.end(); ++i )
    {  
        const Data& data = *i;
        totalTime += data.time;
    }

    const Compound* compound = getCompound();
    const CompoundVector& children = compound->getChildren();
    float nResources( 0.f );
    for( CompoundVector::const_iterator i = children.begin(); 
         i != children.end(); ++i )
    {
        nResources += (*i)->getUsage();
    }

    const float     timeLeft   = totalTime / nResources;
    EQLOG( LOG_LB ) << "Render time " << totalTime << ", per resource "
                    << timeLeft << ", " << nResources << " resources" << endl;

    const float leftover = _assignTargetTimes( _tree, totalTime, timeLeft );
    if( leftover > 2.f * totalTime * numeric_limits< float >::epsilon( ))
    {
        EQWARN << "Load balancer failure: " << leftover
               << "ms not assigned for next frame" << endl;
    }

    EQASSERTINFO( leftover <= 2.f*totalTime * numeric_limits<float>::epsilon(),
                  leftover << " > " << 
                  2.f * totalTime * numeric_limits< float >::epsilon( ));

    _computeSplit( _tree, sortedData, eq::Viewport(), eq::Range() );
}

float LoadEqualizer::_assignTargetTimes( Node* node, const float totalTime, 
                                         const float resourceTime )
{
    const Compound* compound = node->compound;
    if( compound )
    {
        float time = resourceTime * compound->getUsage(); // default

        EQASSERT( _damping >= 0.f );
        EQASSERT( _damping <= 1.f );

        const LBFrameData&  frameData = _history.front();
        const LBDataVector& items     = frameData.second;
        for( LBDataVector::const_iterator i = items.begin(); 
             i != items.end(); ++i )
        {
            const Data& data = *i;
            const Compound* candidate = data.compound;

            if( compound != candidate )
                continue;

            // found our last rendering time -> use this to smoothen the change:
            time = (1.f - _damping) * time + _damping * data.time;
            break;
        }

        node->time = EQ_MIN( time, totalTime );
        EQLOG( LOG_LB ) << compound->getChannel()->getName() << " usage " 
                        << compound->getUsage() << " target " << node->time
                        << ", left " << totalTime - node->time << std::endl;

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

void LoadEqualizer::_computeSplit( Node* node, LBDataVector* sortedData,
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

        if( !vp.hasArea() || !range.hasData( )) // will not render
            data.time = 0.f;

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
            LBDataVector workingSet = sortedData[ MODE_VERTICAL ];

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
            const Compound* root    = getCompound();
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

        case MODE_HORIZONTAL:
        {
            EQASSERT( range == eq::Range::ALL );
            float        timeLeft = node->left->time;
            float        splitPos = vp.y;
            LBDataVector workingSet = sortedData[ MODE_HORIZONTAL ];

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

            const Compound* root    = getCompound();
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

        case MODE_DB:
        {
            EQASSERT( vp == eq::Viewport::FULL );
            float          timeLeft = node->left->time;
            float          splitPos = range.start;
            LBDataVector workingSet = sortedData[ MODE_DB ];

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

ostream& operator << ( ostream& os, const LoadEqualizer::Node* node )
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

std::ostream& operator << ( std::ostream& os, 
                            const LoadEqualizer::Mode mode )
{
    os << ( mode == LoadEqualizer::MODE_2D         ? "2D" :
            mode == LoadEqualizer::MODE_VERTICAL   ? "VERTICAL" :
            mode == LoadEqualizer::MODE_HORIZONTAL ? "HORIZONTAL" :
            mode == LoadEqualizer::MODE_DB         ? "DB" : "ERROR" );
    return os;
}

std::ostream& operator << ( std::ostream& os, const LoadEqualizer* lb )
{
    if( !lb )
        return os;

    os << disableFlush
       << "load_equalizer" << endl
       << '{' << endl
       << "    mode    " << lb->getMode() << endl;
  
    if( lb->getDamping() != 0.5f )
        os << "    damping " << lb->getDamping() << endl;

    os << '}' << endl << enableFlush;
    return os;
}

}
}
