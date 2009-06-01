
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
        EQLOG( LOG_LB2 ) << "LB tree: " << _tree;
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
            Data& data = *j;
            if( data.channel != channel )
                continue;

            // Found corresponding historical data item
            const uint32_t taskID = data.taskID;
            EQASSERTINFO( taskID > 0, channel->getName( ));

            if( data.vp.getArea() <= 0.f )
                return;

            // gather relevant load data
            int64_t startTime = numeric_limits< int64_t >::max();
            int64_t endTime   = 0;
            bool    loadSet   = false;

            for( uint32_t k = 0; k < nStatistics && !loadSet; ++k )
            {
                const eq::Statistic& stat = statistics[k];
                if( stat.task != taskID ) // from different compound
                    continue;

                switch( stat.type )
                {
                    case eq::Statistic::CHANNEL_CLEAR:
                    case eq::Statistic::CHANNEL_DRAW:
                    case eq::Statistic::CHANNEL_READBACK:
                        startTime = EQ_MIN( startTime, stat.startTime );
                        endTime   = EQ_MAX( endTime, stat.endTime );
                        break;
                
                    // assemble blocks on input frames, stop using subsequent
                    // data
                    case eq::Statistic::CHANNEL_ASSEMBLE:
                        loadSet = true;
                        break;
                
                    default:
                        break;
                }
            }
    
            if( startTime == numeric_limits< int64_t >::max( ))
                return;
    
            data.time = endTime - startTime;
            data.time = EQ_MAX( data.time, 1 );
            data.load = static_cast< float >( data.time ) / data.vp.getArea();
            EQLOG( LOG_LB2 ) << "Added load " << data.load << " (t=" << data.time
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

            if( data.time < 0 )
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
        
        Data& data = items.front();
        data.time = 1;
        data.load = 1.f;
        EQASSERT( data.taskID == 0 );
        EQASSERT( data.channel == 0 );
    }
}

void LoadEqualizer::_computeSplit()
{
    EQASSERT( !_history.empty( ));
    
    const LBFrameData& frameData = _history.front();
    const Compound* compound = getCompound();
    EQLOG( LOG_LB2 ) << "----- balance " << compound->getChannel()->getName()
                    << " using frame " << frameData.first << std::endl;

    // sort load items for each of the split directions
    LBDataVector items( frameData.second );
    _removeEmpty( items );

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

        LBDataVector& yData = sortedData[ MODE_HORIZONTAL ];
        sort( yData.begin(), yData.end(), _compareY );

#ifndef NDEBUG
        for( LBDataVector::const_iterator i = xData.begin(); i != xData.end();
             ++i )
        {  
            const Data& data = *i;
            EQLOG( LOG_LB2 ) << "  " << data.vp << ", load " << data.load 
                            << " (t=" << data.load * data.vp.getArea() << ")"
                            << std::endl;
        }
#endif
    }


    // Compute total rendering time
    int64_t totalTime = 0;
    for( LBDataVector::const_iterator i = items.begin(); i != items.end(); ++i )
    {  
        const Data& data = *i;
        totalTime += data.time;
    }

    const CompoundVector& children = compound->getChildren();
    float nResources( 0.f );
    for( CompoundVector::const_iterator i = children.begin(); 
         i != children.end(); ++i )
    {
        nResources += (*i)->getUsage();
    }

    const float timeLeft = static_cast< float >( totalTime ) /
                           static_cast< float >( nResources );
    EQLOG( LOG_LB2 ) << "Render time " << totalTime << ", per resource "
                    << timeLeft << ", " << nResources << " resources" << endl;

    const float leftover = _assignTargetTimes( _tree, totalTime, timeLeft );
    _assignLeftoverTime( _tree, leftover );
    _computeSplit( _tree, sortedData, eq::Viewport(), eq::Range() );
}

float LoadEqualizer::_assignTargetTimes( Node* node, const float totalTime, 
                                         const float resourceTime )
{
    const Compound* compound = node->compound;
    if( compound )
    {
        const float usage = compound->getUsage();
        float time = resourceTime * usage;

        if( usage > 0.0f )
        {
            EQASSERT( _damping >= 0.f );
            EQASSERT( _damping <= 1.f );

            const LBFrameData&  frameData = _history.front();
            const LBDataVector& items     = frameData.second;
            for( LBDataVector::const_iterator i = items.begin(); 
                 i != items.end(); ++i )
            {
                const Data& data = *i;
                const uint32_t taskID = data.taskID;
                
                if( compound->getTaskID() != taskID )
                    continue;

                // found our last rendering time -> use it to smooth the change:
                time = (1.f - _damping) * time + _damping * data.time;
                break;
            }
        }

        node->time  = EQ_MIN( time, totalTime );
        node->usage = usage;
        EQLOG( LOG_LB2 ) << compound->getChannel()->getName() << " usage " 
                        << compound->getUsage() << " target " << node->time
                        << ", left " << totalTime - node->time << std::endl;

        return totalTime - node->time;
    }

    EQASSERT( node->left );
    EQASSERT( node->right );

    float timeLeft = _assignTargetTimes( node->left, totalTime, resourceTime );
    timeLeft       = _assignTargetTimes( node->right, timeLeft, resourceTime );
    node->time  = node->left->time + node->right->time;
    node->usage = node->left->usage + node->right->usage;
    
    EQLOG( LOG_LB2 ) << "Node time " << node->time << ", left " << timeLeft
                    << endl;
    return timeLeft;
}

void LoadEqualizer::_assignLeftoverTime( Node* node, const float time )
{
    const Compound* compound = node->compound;
    if( compound )
    {
        node->time += time;
        EQLOG( LOG_LB2 ) << compound->getChannel()->getName() << " usage " 
                        << compound->getUsage() << " target " << node->time
                        << std::endl;
        EQASSERTINFO( node->usage > 0.0f || node->time <= 0.f,
                      node->usage << ", " << node->time );
    }
    else
    {
        EQASSERT( node->left );
        EQASSERT( node->right );

        if( node->usage > 0.f )
        {
            float leftTime = time * node->left->usage / node->usage;
            float rightTime = time - leftTime;
            if( time - leftTime < 0.0001f )
            {
                leftTime = time;
                rightTime = 0.f;
            }
            else if( time - rightTime < 0.0001f )
            {
                leftTime = 0.f;
                rightTime = time;
            }

            _assignLeftoverTime( node->left, leftTime );
            _assignLeftoverTime( node->right, rightTime );
            node->time = node->left->time + node->right->time;
        }
        else
        {
            EQASSERTINFO( time <= 10.f * std::numeric_limits<float>::epsilon(),
                          time );
        }
    }
}

void LoadEqualizer::_removeEmpty( LBDataVector& items )
{
    for( LBDataVector::iterator i = items.begin(); i != items.end(); )
    {  
        Data& data = *i;

        if( !data.vp.hasArea() || !data.range.hasData( ))
            i = items.erase( i );
        else
            ++i;
    }
}

void LoadEqualizer::_computeSplit( Node* node, LBDataVector* sortedData,
                                   const eq::Viewport& vp,
                                   const eq::Range& range )
{
    const float time = node->time;
    EQLOG( LOG_LB2 ) << "_computeSplit " << vp << ", " << range << " time "
                    << time << endl;
    EQASSERTINFO( vp.isValid(), vp );
    EQASSERTINFO( range.isValid(), range );
    EQASSERTINFO( node->usage > 0.f || !vp.hasArea() || !range.hasData(),
                  "Assigning work to unused compound" );

    Compound* compound = node->compound;
    if( compound )
    {
        EQASSERTINFO( vp == eq::Viewport::FULL || range == eq::Range::ALL,
                      "Mixed 2D/DB load-balancing not implemented" );

        // TODO: check that time == vp * load
        compound->setViewport( vp );
        compound->setRange( range );

        EQLOG( LOG_LB2 ) << compound->getChannel()->getName() << " set "
                        << vp << ", " << range << endl;

        // save data for later use
        Data data;
        data.vp      = vp;
        data.range   = range;
        data.channel = compound->getChannel();
        data.taskID  = compound->getTaskID();
        EQASSERT( data.taskID > 0 );

        if( !vp.hasArea() || !range.hasData( )) // will not render
            data.time = 0;

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
            const float    end      = vp.getXEnd();
            LBDataVector workingSet = sortedData[ MODE_VERTICAL ];

            while( timeLeft > std::numeric_limits< float >::epsilon() &&
                   splitPos < end && !workingSet.empty())
            {
                EQLOG( LOG_LB2 ) << timeLeft << "ms left for "
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
                EQLOG( LOG_LB2 ) << "Computing load in X " << splitPos << "..."
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
                        const float percentage = yContrib / vp.h;
                        EQLOG( LOG_LB2 ) << data.vp << " contributes "
                                        << yContrib << " of " << data.vp.h
                                        << " (" << percentage
                                        << ") with " << data.load << ": "
                                        << ( data.load * percentage )
                                        << " vp.y " << vp.y << " dataEnd " 
                                        << dataEnd << " vpEnd " << vpEnd
                                        << endl;

                        currentLoad += ( data.load * percentage );
                    }
                }

                const float width        = currentPos - splitPos;
                const float area         = width * vp.h;
                const float currentTime  = area * currentLoad;
                    
                EQLOG( LOG_LB2 ) << splitPos << "..." << currentPos 
                                << ": t=" << currentTime << " of " 
                                << timeLeft << endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    splitPos += (width * timeLeft / currentTime );
                    timeLeft = 0.0f;
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                }
            }

            EQLOG( LOG_LB2 ) << "Should split at X " << splitPos << endl;
            // There might be more time left due to MIN_PIXEL rounding by parent
            // EQASSERTINFO( timeLeft <= .001f, timeLeft );

            // Ensure minimum size
            const Compound* root = getCompound();
            if( node->left->usage == 0.f )
                splitPos = vp.x;
            else if( node->right->usage == 0.f )
                splitPos = end;
#ifdef MIN_PIXELS
            else
            {
                const float     epsilon = static_cast< float >( MIN_PIXELS ) /
                                              root->getInheritPixelViewport().w;

                if( (splitPos - vp.x) < epsilon )
                    splitPos = vp.x + epsilon;
                if( (end - splitPos) < epsilon )
                    splitPos = end - epsilon;
            }
#endif
            const float epsilon = 1.0f / root->getInheritPixelViewport().w;
            if( (splitPos - vp.x) < epsilon )
                splitPos = vp.x;
            if( (end - splitPos) < epsilon )
                splitPos = end;

            splitPos = EQ_MAX( splitPos, vp.x );
            splitPos = EQ_MIN( splitPos, end);

            EQLOG( LOG_LB2 ) << "Split " << vp << " at X " << splitPos << endl;

            // balance children
            eq::Viewport childVP = vp;
            childVP.w = (splitPos - vp.x);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.x = childVP.getXEnd();
            childVP.w = end - childVP.x;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case MODE_HORIZONTAL:
        {
            if( node->left->usage > 0.f && node->right->usage > 0.f )

            EQASSERT( range == eq::Range::ALL );
            float        timeLeft = node->left->time;
            float        splitPos = vp.y;
            const float  end      = vp.getYEnd();
            LBDataVector workingSet = sortedData[ MODE_HORIZONTAL ];

            while( timeLeft > std::numeric_limits< float >::epsilon() &&
                   splitPos < end && !workingSet.empty( ))
            {
                EQLOG( LOG_LB2 ) << timeLeft << "ms left for "
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
                EQLOG( LOG_LB2 ) << "Computing load in Y " << splitPos << "..."
                                << currentPos << endl;
                float currentLoad = 0.f;
                for( LBDataVector::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                        
                    if( data.vp.y >= currentPos ) // not yet needed data sets
                        break;

                    float xContrib = data.vp.w;

                    if( data.vp.x < vp.x )
                        xContrib -= (vp.x - data.vp.x);
                    
                    const float dataEnd = data.vp.getXEnd();
                    const float vpEnd   = vp.getXEnd();
                    if( dataEnd > vpEnd )
                        xContrib -= (dataEnd - vpEnd);
                    
                    if( xContrib > 0.f )
                    {
                        const float percentage = xContrib / vp.w;
                        EQLOG( LOG_LB2 ) << data.vp << " contributes "
                                        << xContrib << " of " << data.vp.w
                                        << " (" << percentage
                                        << ") with " << data.load << ": "
                                        << ( data.load * percentage )
                                        << " vp.x " << vp.x << " dataEnd " 
                                        << dataEnd << " vpEnd " << vpEnd
                                        << endl;

                        currentLoad += ( data.load * percentage );
                    }
                }

                const float height       = currentPos - splitPos;
                const float area         = height * vp.w;
                const float currentTime  = area * currentLoad;
                    
                EQLOG( LOG_LB2 ) << splitPos << "..." << currentPos 
                                << ": t=" << currentTime << " of " 
                                << timeLeft << endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    splitPos += (height * timeLeft / currentTime );
                    timeLeft = 0.0f;
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                }
            }

            EQLOG( LOG_LB2 ) << "Should split at Y " << splitPos << endl;
            // There might be more time left due to MIN_PIXEL rounding by parent
            // EQASSERTINFO( timeLeft <= .001f, timeLeft );

            const Compound* root = getCompound();
            if( node->left->usage == 0.f )
                splitPos = vp.y;
            else if( node->right->usage == 0.f )
                splitPos = end;
#ifdef MIN_PIXELS
            else
            {
                const float     epsilon = static_cast< float >( MIN_PIXELS ) /
                                              root->getInheritPixelViewport().h;

                if( (splitPos - vp.y) < epsilon )
                    splitPos = vp.y + epsilon;
                if( (end - splitPos) < epsilon )
                    splitPos = end - epsilon;
            }
#endif
            const float epsilon = 1.0f / root->getInheritPixelViewport().h;
            if( (splitPos - vp.y) < epsilon )
                splitPos = vp.y;
            if( (end - splitPos) < epsilon )
                splitPos = end;

            splitPos = EQ_MAX( splitPos, vp.y );
            splitPos = EQ_MIN( splitPos, end );

            EQLOG( LOG_LB2 ) << "Split " << vp << " at Y " << splitPos << endl;

            eq::Viewport childVP = vp;
            childVP.h = (splitPos - vp.y);
            _computeSplit( node->left, sortedData, childVP, range );

            childVP.y = childVP.getYEnd();
            childVP.h = end - childVP.y;
            _computeSplit( node->right, sortedData, childVP, range );
            break;
        }

        case MODE_DB:
        {
            EQASSERT( vp == eq::Viewport::FULL );
            float          timeLeft = node->left->time;
            float          splitPos = range.start;
            const float    end      = range.end;
            LBDataVector workingSet = sortedData[ MODE_DB ];

            while( timeLeft > std::numeric_limits< float >::epsilon() && 
                   splitPos < end && !workingSet.empty( ))
            {
                EQLOG( LOG_LB2 ) << timeLeft << "ms left for "
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
                EQLOG( LOG_LB2 ) << "Computing load in range " << splitPos
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

                EQLOG( LOG_LB2 ) << splitPos << "..." << currentPos 
                                << ": t=" << currentLoad << " of " 
                                << timeLeft << endl;

                if( currentLoad >= timeLeft ) // found last region
                {
                    const float width = currentPos - splitPos;
                    splitPos += (width * timeLeft / currentLoad );
                    timeLeft = 0.0f;
                }
                else
                {
                    timeLeft -= currentLoad;
                    splitPos  = currentPos;
                }
            }
            // There might be more time left due to MIN_PIXEL rounding by parent
            // EQASSERTINFO( timeLeft <= .001f, timeLeft );

            if( node->left->usage == 0.f )
                splitPos = range.start;
            else if( node->right->usage == 0.f )
                splitPos = end;

            const float epsilon( 0.001f );
            if( (splitPos - range.start) < epsilon )
                splitPos = range.start;
            if( (end - splitPos) < epsilon )
                splitPos = end;

            EQLOG( LOG_LB2 ) << "Split " << range << " at pos " << splitPos
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
