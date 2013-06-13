
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/client/statistic.h>
#include <lunchbox/debug.h>

namespace eq
{
namespace server
{

std::ostream& operator << ( std::ostream& os, const LoadEqualizer::Node* );

// The tree load balancer organizes the children in a binary tree. At each
// level, a relative split position is determined by balancing the left subtree
// against the right subtree.

LoadEqualizer::LoadEqualizer()
        : _tree( 0 )
{
    LBVERB << "New LoadEqualizer @" << (void*)this << std::endl;
}

LoadEqualizer::LoadEqualizer( const fabric::Equalizer& from )
        : Equalizer( from )
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
    _checkHistory(); // execute to not leak memory
    if( isFrozen() || !compound->isActive() || !isActive( ))
        return;

    if( !_tree )
    {
        LBASSERT( compound == getCompound( ));
        const Compounds& children = compound->getChildren();
        switch( children.size( ))
        {
          case 0: return; // no child compounds, can't do anything.
          case 1: // one child, 'balance' it:
              if( getMode() == MODE_DB )
                  children.front()->setRange( Range( ));
              else
                  children.front()->setViewport( Viewport( ));
              return;

          default:
              _tree = _buildTree( children );
              break;
        }
    }

    // compute new data
    if( getDamping() < 1.f )
    {
        _history.push_back( LBFrameData( ));
        _history.back().first = frameNumber;
    }

    _update( _tree, Viewport(), Range( ));
    _computeSplit();
}

LoadEqualizer::Node* LoadEqualizer::_buildTree( const Compounds& compounds )
{
    Node* node = new Node;

    const size_t size = compounds.size();
    if( size == 1 )
    {
        Compound* compound = compounds.front();

        node->compound = compound;

        Channel* channel = compound->getChannel();
        LBASSERT( channel );
        channel->addListener( this );
        return node;
    }

    const size_t middle = size >> 1;

    Compounds left;
    for( size_t i = 0; i < middle; ++i )
        left.push_back( compounds[i] );

    Compounds right;
    for( size_t i = middle; i < size; ++i )
        right.push_back( compounds[i] );

    node->left  = _buildTree( left );
    node->right = _buildTree( right );

    return node;
}

void LoadEqualizer::_clearTree( Node* node )
{
    if( !node )
        return;

    if( node->compound )
    {
        Channel* channel = node->compound->getChannel();
        LBASSERTINFO( channel, node->compound );
        channel->removeListener( this );
    }
    else
    {
        _clearTree( node->left );
        _clearTree( node->right );
    }
}

void LoadEqualizer::notifyLoadData( Channel* channel,
                                    const uint32_t frameNumber,
                                    const Statistics& statistics,
                                    const Viewport& region )
{
    LBLOG( LOG_LB2 ) << statistics.size()
                     << " samples from "<< channel->getName()
                     << " @ " << frameNumber << std::endl;
    for( std::deque< LBFrameData >::iterator i = _history.begin();
         i != _history.end(); ++i )
    {
        LBFrameData& frameData = *i;
        if( frameData.first != frameNumber )
            continue;

        // Found corresponding historical data set
        LBDatas& items = frameData.second;
        for( LBDatas::iterator j = items.begin(); j != items.end(); ++j )
        {
            Data& data = *j;
            if( data.channel != channel )
                continue;

            // Found corresponding historical data item
            const uint32_t taskID = data.taskID;
            LBASSERTINFO( taskID > 0, channel->getName( ));

            // gather relevant load data
            int64_t startTime = std::numeric_limits< int64_t >::max();
            int64_t endTime   = 0;
            bool    loadSet   = false;
            int64_t transmitTime = 0;
            for( size_t k = 0; k < statistics.size(); ++k )
            {
                const Statistic& stat = statistics[k];
                if( stat.task == data.destTaskID )
                    _updateAssembleTime( data, stat );

                // from different compound
                if( stat.task != taskID || loadSet )
                    continue;

                switch( stat.type )
                {
                case Statistic::CHANNEL_CLEAR:
                case Statistic::CHANNEL_DRAW:
                case Statistic::CHANNEL_READBACK:
                    startTime = LB_MIN( startTime, stat.startTime );
                    endTime   = LB_MAX( endTime, stat.endTime );
                    break;

                case Statistic::CHANNEL_ASYNC_READBACK:
                case Statistic::CHANNEL_FRAME_TRANSMIT:
                    transmitTime += stat.endTime - stat.startTime;
                    break;
                case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
                    transmitTime -= stat.endTime - stat.startTime;
                    break;

                // assemble blocks on input frames, stop using subsequent data
                case Statistic::CHANNEL_ASSEMBLE:
                    loadSet = true;
                    break;

                default:
                    break;
                }
            }

            if( startTime == std::numeric_limits< int64_t >::max( ))
                return;

            data.vp.apply( region ); // Update ROI
            data.time = endTime - startTime;
            data.time = LB_MAX( data.time, 1 );
            data.time = LB_MAX( data.time, transmitTime );
            data.assembleTime = LB_MAX( data.assembleTime, 0 );
            LBLOG( LOG_LB2 ) << "Added time " << data.time << " (+"
                             << data.assembleTime << ") for "
                             << channel->getName() << " " << data.vp << ", "
                             << data.range << " @ " << frameNumber << std::endl;
            return;

            // Note: if the same channel is used twice as a child, the
            // load-compound association does not work.
        }
    }
}

void LoadEqualizer::_updateAssembleTime( Data& data, const Statistic& stat )
{
    switch( stat.type )
    {
        case Statistic::CHANNEL_FRAME_WAIT_READY:
            data.assembleTime -= stat.endTime - stat.startTime;
            break;

        case Statistic::CHANNEL_ASSEMBLE:
            data.assembleTime += stat.endTime - stat.startTime;
            break;

        default:
            break;
    }
}

void LoadEqualizer::_checkHistory()
{
    // 1. Find youngest complete load data set
    uint32_t useFrame = 0;
    for( std::deque< LBFrameData >::reverse_iterator i = _history.rbegin();
         i != _history.rend() && useFrame == 0; ++i )
    {
        const LBFrameData& frameData = *i;
        const LBDatas& items = frameData.second;
        bool isComplete = true;

        for( LBDatas::const_iterator j = items.begin();
             j != items.end() && isComplete; ++j )
        {
            const Data& data = *j;

            if( data.time < 0 )
                isComplete = false;
        }

        if( isComplete )
            useFrame = frameData.first;
    }

    // 2. delete old, unneeded data sets
    while( !_history.empty() && _history.front().first < useFrame )
        _history.pop_front();

    if( _history.empty( )) // insert fake set
    {
        _history.resize( 1 );

        LBFrameData&  frameData  = _history.front();
        LBDatas& items      = frameData.second;

        frameData.first = 0; // frameNumber
        items.resize( 1 );

        Data& data = items.front();
        data.time = 1;
        LBASSERT( data.taskID == 0 );
        LBASSERT( data.channel == 0 );
    }
}

float LoadEqualizer::_getTotalResources( ) const
{
    const Compounds& children = getCompound()->getChildren();

    float resources = 0.f;
    for( CompoundsCIter i = children.begin(); i != children.end(); ++i )
    {
       const Compound* compound = *i;
       if( compound->isActive( ))
           resources += compound->getUsage();
    }

    return resources;
}

void LoadEqualizer::_update( Node* node, const Viewport& vp,
                             const Range& range )
{
    if( !node )
        return;

    node->mode = getMode();
    if( node->mode == MODE_2D )
    {
        PixelViewport pvp = getCompound()->getChannel()->getPixelViewport();
        pvp.apply( vp );

        if( pvp.w > pvp.h ) // split along longest axis
            node->mode = MODE_VERTICAL;
        else
            node->mode = MODE_HORIZONTAL;
    }

    if( node->compound )
        _updateLeaf( node );
    else
        _updateNode( node, vp, range );
}

void LoadEqualizer::_updateLeaf( Node* node )
{
    const Compound* compound = node->compound;
    const Channel* channel = compound->getChannel();
    LBASSERT( channel );
    const PixelViewport& pvp = channel->getPixelViewport();
    node->resources = compound->isActive() ? compound->getUsage() : 0.f;
    LBLOG( LOG_LB2 ) << channel->getName() << " active " << compound->isActive()
                     << " using " << node->resources << std::endl;
    LBASSERT( node->resources >= 0.f );

    node->maxSize.x() = pvp.w;
    node->maxSize.y() = pvp.h;
    node->boundaryf = getBoundaryf();
    node->boundary2i = getBoundary2i();
    node->resistancef = getResistancef();
    node->resistance2i = getResistance2i();
    if( !compound->hasDestinationChannel( ))
        return;

    const float nResources = _getTotalResources();
    if( getAssembleOnlyLimit() <= nResources - node->resources )
    {
        node->resources = 0.f;
        return; // OPT
    }

    const float time = float( _getTotalTime( ));
    const float assembleTime = float( _getAssembleTime( ));
    if( assembleTime == 0.f || node->resources == 0.f )
        return;

    const float timePerResource = time / ( nResources - node->resources );
    const float renderTime = timePerResource * node->resources ;

    const float clampedAssembleTime = LB_MIN( assembleTime, renderTime );
    const float newTimePerResource = (time + clampedAssembleTime) / nResources;
    node->resources -= ( clampedAssembleTime / newTimePerResource );
    if( node->resources < 0.f ) // may happen due to fp rounding
        node->resources = 0.f;
}

void LoadEqualizer::_updateNode( Node* node, const Viewport& vp,
                                 const Range& range )
{
    Node* left = node->left;
    Node* right = node->right;

    LBASSERT( left );
    LBASSERT( right );

    Viewport leftVP = vp;
    Viewport rightVP = vp;
    Range leftRange = range;
    Range rightRange = range;

    switch( node->mode )
    {
      default:
        LBUNIMPLEMENTED;

      case MODE_VERTICAL:
        leftVP.w = vp.w * .5f;
        rightVP.x = leftVP.getXEnd();
        rightVP.w = vp.getXEnd() - rightVP.x;
        node->split = leftVP.getXEnd();
        break;

      case MODE_HORIZONTAL:
        leftVP.h = vp.h * .5f;
        rightVP.y = leftVP.getYEnd();
        rightVP.h = vp.getYEnd() - rightVP.y;
        node->split = leftVP.getYEnd();
        break;

      case MODE_DB:
        leftRange.end = range.start + ( range.end - range.start ) * .5f;
        rightRange.start = leftRange.end;
        node->split = leftRange.end;
        break;
    }

    _update( left, leftVP, leftRange );
    _update( right, rightVP, rightRange );

    node->resources = left->resources + right->resources;

    if( left->resources == 0.f )
    {
        node->maxSize    = right->maxSize;
        node->boundary2i = right->boundary2i;
        node->boundaryf  = right->boundaryf;
        node->resistance2i = right->resistance2i;
        node->resistancef = right->resistancef;
    }
    else if( right->resources == 0.f )
    {
        node->maxSize = left->maxSize;
        node->boundary2i = left->boundary2i;
        node->boundaryf = left->boundaryf;
        node->resistance2i = left->resistance2i;
        node->resistancef = left->resistancef;
    }
    else
    {
        switch( node->mode )
        {
        case MODE_VERTICAL:
            node->maxSize.x() = left->maxSize.x() + right->maxSize.x();
            node->maxSize.y() = LB_MIN( left->maxSize.y(), right->maxSize.y());
            node->boundary2i.x() = left->boundary2i.x()+ right->boundary2i.x();
            node->boundary2i.y() = LB_MAX( left->boundary2i.y(),
                                           right->boundary2i.y());
            node->boundaryf = LB_MAX( left->boundaryf, right->boundaryf );
            node->resistance2i.x() = LB_MAX( left->resistance2i.x(),
                                             right->resistance2i.x( ));
            node->resistance2i.y() = LB_MAX( left->resistance2i.y(),
                                             right->resistance2i.y());
            node->resistancef = LB_MAX( left->resistancef, right->resistancef );
            break;
        case MODE_HORIZONTAL:
            node->maxSize.x() = LB_MIN( left->maxSize.x(), right->maxSize.x());
            node->maxSize.y() = left->maxSize.y() + right->maxSize.y();
            node->boundary2i.x() = LB_MAX( left->boundary2i.x(),
                                           right->boundary2i.x() );
            node->boundary2i.y() = left->boundary2i.y()+ right->boundary2i.y();
            node->boundaryf = LB_MAX( left->boundaryf, right->boundaryf );
            node->resistance2i.x() = LB_MAX( left->resistance2i.x(),
                                             right->resistance2i.x() );
            node->resistance2i.y() = LB_MAX( left->resistance2i.y(),
                                             right->resistance2i.y( ));
            node->resistancef = LB_MAX( left->resistancef, right->resistancef );
            break;
        case MODE_DB:
            node->boundary2i.x() = LB_MAX( left->boundary2i.x(),
                                           right->boundary2i.x() );
            node->boundary2i.y() = LB_MAX( left->boundary2i.y(),
                                           right->boundary2i.y() );
            node->boundaryf = left->boundaryf + right->boundaryf;
            node->resistance2i.x() = LB_MAX( left->resistance2i.x(),
                                             right->resistance2i.x() );
            node->resistance2i.y() = LB_MAX( left->resistance2i.y(),
                                             right->resistance2i.y() );
            node->resistancef = LB_MAX( left->resistancef, right->resistancef );
            break;
        default:
            LBUNIMPLEMENTED;
        }
    }
}

int64_t LoadEqualizer::_getTotalTime()
{
    const LBFrameData& frameData = _history.front();
    LBDatas items = frameData.second;
    _removeEmpty( items );

    int64_t totalTime = 0;
    for( LBDatas::const_iterator i = items.begin(); i != items.end(); ++i )
    {
        const Data& data = *i;
        totalTime += data.time;
    }
    return totalTime;
}

int64_t LoadEqualizer::_getAssembleTime( )
{
    if( getDamping() >= 1.f )
        return 0;

    const LBFrameData& frameData = _history.front();
    const LBDatas& items = frameData.second;

    int64_t assembleTime = 0;
    for( LBDatas::const_iterator i = items.begin(); i != items.end(); ++i )
    {
        const Data& data = *i;
        LBASSERT( assembleTime == 0 || data.assembleTime == 0 );
        assembleTime += data.assembleTime;
    }
    return assembleTime;
}

void LoadEqualizer::_computeSplit()
{
    LBASSERT( !_history.empty( ));

    const LBFrameData& frameData = _history.front();
    const Compound* compound = getCompound();
    LBLOG( LOG_LB2 ) << "----- balance " << compound->getChannel()->getName()
                    << " using frame " << frameData.first << " tree "
                     << std::endl << _tree;

    // sort load items for each of the split directions
    LBDatas items( frameData.second );
    _removeEmpty( items );

    LBDatas sortedData[3] = { items, items, items };

    if( getMode() == MODE_DB )
    {
        LBDatas& rangeData = sortedData[ MODE_DB ];
        sort( rangeData.begin(), rangeData.end(), _compareRange );
    }
    else
    {
        LBDatas& xData = sortedData[ MODE_VERTICAL ];
        sort( xData.begin(), xData.end(), _compareX );

        LBDatas& yData = sortedData[ MODE_HORIZONTAL ];
        sort( yData.begin(), yData.end(), _compareY );

#ifndef NDEBUG
        for( LBDatas::const_iterator i = xData.begin(); i != xData.end();
             ++i )
        {
            const Data& data = *i;
            LBLOG( LOG_LB2 ) << "  " << data.vp << ", time " << data.time
                             << " (+" << data.assembleTime << ")" << std::endl;
        }
#endif
    }

    const float time = float( _getTotalTime( ));
    LBLOG( LOG_LB2 ) << "Render time " << time << " for "
                     << _tree->resources << " resources" << std::endl;
    if( _tree->resources > 0.f )
        _computeSplit( _tree, time, sortedData, Viewport(), Range( ));
}

void LoadEqualizer::_removeEmpty( LBDatas& items )
{
    for( LBDatas::iterator i = items.begin(); i != items.end(); )
    {
        Data& data = *i;

        if( !data.vp.hasArea() || !data.range.hasData( ))
            i = items.erase( i );
        else
            ++i;
    }
}

void LoadEqualizer::_computeSplit( Node* node, const float time,
                                   LBDatas* datas, const Viewport& vp,
                                   const Range& range )
{
    LBLOG( LOG_LB2 ) << "_computeSplit " << vp << ", " << range << " time "
                    << time << std::endl;
    LBASSERTINFO( vp.isValid(), vp );
    LBASSERTINFO( range.isValid(), range );
    LBASSERTINFO( node->resources > 0.f || !vp.hasArea() || !range.hasData(),
                  "Assigning " << node->resources <<
                  " work to viewport " << vp << ", " << range );

    Compound* compound = node->compound;
    if( compound )
    {
        _assign( compound, vp, range );
        return;
    }

    LBASSERT( node->left && node->right );

    LBDatas workingSet = datas[ node->mode ];
    const float leftTime = node->resources > 0 ?
                           time * node->left->resources / node->resources : 0.f;
    float timeLeft = LB_MIN( leftTime, time ); // correct for fp rounding error

    switch( node->mode )
    {
        case MODE_VERTICAL:
        {
            LBASSERT( range == Range::ALL );

            float splitPos = vp.x;
            const float end = vp.getXEnd();

            while( timeLeft > std::numeric_limits< float >::epsilon() &&
                   splitPos < end )
            {
                LBLOG( LOG_LB2 ) << timeLeft << "ms left using "
                                << workingSet.size() << " tiles" << std::endl;

                // remove all irrelevant items from working set
                for( LBDatas::iterator i = workingSet.begin();
                     i != workingSet.end(); )
                {
                    const Data& data = *i;
                    if( data.vp.getXEnd() > splitPos )
                        ++i;
                    else
                        i = workingSet.erase( i );
                }
                if( workingSet.empty( ))
                    break;

                // find next 'discontinouity' in loads
                float currentPos = 1.0f;
                for( LBDatas::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    if( data.vp.x > splitPos && data.vp.x < currentPos )
                        currentPos = data.vp.x;
                    const float xEnd = data.vp.getXEnd();
                    if( xEnd > splitPos && xEnd < currentPos )
                        currentPos = xEnd;
                }

                const float width = currentPos - splitPos;
                LBASSERTINFO( width > 0.f, currentPos << "<=" << splitPos );
                LBASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                LBLOG( LOG_LB2 ) << "Computing load in X " << splitPos << "..."
                                 << currentPos << std::endl;
                float currentTime = 0.f;
                for( LBDatas::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;

                    if( data.vp.x >= currentPos ) // not yet needed data sets
                        break;

                    float yContrib = data.vp.h;
                    if( data.vp.y < vp.y )
                        yContrib -= (vp.y - data.vp.y);

                    const float dataEnd = data.vp.getYEnd();
                    const float vpEnd   = vp.getYEnd();
                    if( dataEnd > vpEnd )
                        yContrib -= (dataEnd - vpEnd);

                    if( yContrib > 0.f )
                    {
                        const float percentage = ( width / data.vp.w ) *
                                                 ( yContrib / data.vp.h );
                        currentTime += ( data.time * percentage );

                        LBLOG( LOG_LB2 ) << data.vp << " contributes "
                                         << yContrib << " in " << vp.h << " ("
                                         << percentage << ") with " << data.time
                                         << ": " << ( data.time * percentage )
                                         << " vp.y " << vp.y << " dataEnd "
                                         << dataEnd << " vpEnd " << vpEnd
                                         << std::endl;
                        LBASSERT( percentage < 1.01f )
                    }
                }

                LBLOG( LOG_LB2 ) << splitPos << "..." << currentPos << ": t="
                                 << currentTime << " of " << timeLeft
                                 << std::endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    splitPos += ( width * timeLeft / currentTime );
                    timeLeft = 0.0f;
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                }
            }

            LBLOG( LOG_LB2 ) << "Should split at X " << splitPos << std::endl;
            if( getDamping() < 1.f )
                splitPos = (1.f - getDamping()) * splitPos +
                            getDamping() * node->split;
            LBLOG( LOG_LB2 ) << "Dampened split at X " << splitPos << std::endl;

            // There might be more time left due to MIN_PIXEL rounding by parent
            // LBASSERTINFO( timeLeft <= .001f, timeLeft );

            // Ensure minimum size
            const Compound* root = getCompound();
            const float pvpW = static_cast< float >(
                root->getInheritPixelViewport().w );
            const float boundary = static_cast< float >( node->boundary2i.x()) /
                                       pvpW;
            if( node->left->resources == 0.f )
                splitPos = vp.x;
            else if( node->right->resources == 0.f )
                splitPos = end;
            else if( boundary > 0 )
            {
                const float lengthRight = vp.getXEnd() - splitPos;
                const float lengthLeft = splitPos - vp.x;
                const float maxRight =
                    static_cast< float >( node->right->maxSize.x( )) / pvpW;
                const float maxLeft =
                    static_cast< float >( node->left->maxSize.x( )) / pvpW;
                if( lengthRight > maxRight )
                    splitPos = end - maxRight;
                else if( lengthLeft > maxLeft )
                    splitPos = vp.x + maxLeft;

                if( (splitPos - vp.x) < boundary )
                    splitPos = vp.x + boundary;
                if( (end - splitPos) < boundary )
                    splitPos = end - boundary;

                const uint32_t ratio =
                           static_cast< uint32_t >( splitPos / boundary + .5f );
                splitPos = ratio * boundary;
            }

            splitPos = LB_MAX( splitPos, vp.x );
            splitPos = LB_MIN( splitPos, end);

            const float newPixelW = pvpW * splitPos;
            const float oldPixelW = pvpW * node->split;
            if( int( fabs(newPixelW - oldPixelW) ) < node->resistance2i.x( ))
                splitPos = node->split;
            else
                node->split = splitPos;

            LBLOG( LOG_LB2 ) << "Constrained split " << vp << " at X "
                             << splitPos << std::endl;

            // balance children
            Viewport childVP = vp;
            childVP.w = (splitPos - vp.x);
            _computeSplit( node->left, leftTime, datas, childVP, range );

            childVP.x = childVP.getXEnd();
            childVP.w = end - childVP.x;
            // Fix 2994111: Rounding errors with 2D LB and 16 sources
            //   Floating point rounding may create a width for the 'right'
            //   child which is slightly below the parent width. Correct it.
            while( childVP.getXEnd() < end )
                childVP.w += std::numeric_limits< float >::epsilon();
            _computeSplit( node->right, time-leftTime, datas, childVP, range );
            break;
        }

        case MODE_HORIZONTAL:
        {
            LBASSERT( range == Range::ALL );
            float splitPos = vp.y;
            const float end = vp.getYEnd();

            while( timeLeft > std::numeric_limits< float >::epsilon() &&
                   splitPos < end )
            {
                LBLOG( LOG_LB2 ) << timeLeft << "ms left using "
                                 << workingSet.size() << " tiles" << std::endl;

                // remove all unrelevant items from working set
                for( LBDatas::iterator i = workingSet.begin();
                     i != workingSet.end(); )
                {
                    const Data& data = *i;
                    if( data.vp.getYEnd() > splitPos )
                        ++i;
                    else
                        i = workingSet.erase( i );
                }
                if( workingSet.empty( ))
                    break;

                // find next 'discontinuouity' in loads
                float currentPos = 1.0f;
                for( LBDatas::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    if( data.vp.y > splitPos && data.vp.y < currentPos )
                        currentPos = data.vp.y;
                    const float yEnd = data.vp.getYEnd();
                    if( yEnd > splitPos && yEnd < currentPos )
                        currentPos = yEnd;
                }

                const float height = currentPos - splitPos;
                LBASSERTINFO( height > 0.f, currentPos << "<=" << splitPos );
                LBASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                LBLOG( LOG_LB2 ) << "Computing load in Y " << splitPos << "..."
                                << currentPos << std::endl;
                float currentTime = 0.f;
                for( LBDatas::const_iterator i = workingSet.begin();
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
                        const float percentage = ( height / data.vp.h ) *
                                                 ( xContrib / data.vp.w );
                        currentTime += ( data.time * percentage );

                        LBLOG( LOG_LB2 ) << data.vp << " contributes "
                                         << xContrib << " in " << vp.w << " ("
                                         << percentage << ") with " << data.time
                                         << ": " << ( data.time * percentage )
                                         << " total " << currentTime << " vp.x "
                                         << vp.x << " dataEnd " << dataEnd
                                         << " vpEnd " << vpEnd << std::endl;
                        LBASSERT( percentage < 1.01f )
                    }
                }

                LBLOG( LOG_LB2 ) << splitPos << "..." << currentPos << ": t="
                                 << currentTime << " of " << timeLeft
                                 << std::endl;

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

            LBLOG( LOG_LB2 ) << "Should split at Y " << splitPos << std::endl;
            if( getDamping() < 1.f )
                splitPos = (1.f - getDamping( )) * splitPos +
                            getDamping() * node->split;
            LBLOG( LOG_LB2 ) << "Dampened split at Y " << splitPos << std::endl;

            const Compound* root = getCompound();

            const float pvpH = static_cast< float >(
                root->getInheritPixelViewport().h );
            const float boundary = static_cast< float >(node->boundary2i.y( )) /
                                       pvpH;

            if( node->left->resources == 0.f )
                splitPos = vp.y;
            else if( node->right->resources == 0.f )
                splitPos = end;
            else if ( boundary > 0 )
            {
                const float lengthRight = vp.getYEnd() - splitPos;
                const float lengthLeft = splitPos - vp.y;
                const float maxRight =
                    static_cast< float >( node->right->maxSize.y( )) / pvpH;
                const float maxLeft =
                    static_cast< float >( node->left->maxSize.y( )) / pvpH;
                if( lengthRight > maxRight )
                    splitPos = end - maxRight;
                else if( lengthLeft > maxLeft )
                    splitPos = vp.y + maxLeft;

                if( (splitPos - vp.y) < boundary )
                    splitPos = vp.y + boundary;
                if( (end - splitPos) < boundary )
                    splitPos = end - boundary;

                const uint32_t ratio =
                           static_cast< uint32_t >( splitPos / boundary + .5f );
                splitPos = ratio * boundary;
            }

            splitPos = LB_MAX( splitPos, vp.y );
            splitPos = LB_MIN( splitPos, end );

            const float newPixelH = pvpH * splitPos;
            const float oldPixelH = pvpH * node->split;
            if( int( fabs(newPixelH - oldPixelH) ) < node->resistance2i.y( ))
                splitPos = node->split;
            else
                node->split = splitPos;

            LBLOG( LOG_LB2 ) << "Constrained split " << vp << " at Y "
                             << splitPos << std::endl;

            Viewport childVP = vp;
            childVP.h = (splitPos - vp.y);
            _computeSplit( node->left, leftTime, datas, childVP, range );

            childVP.y = childVP.getYEnd();
            childVP.h = end - childVP.y;
            while( childVP.getYEnd() < end )
                childVP.h += std::numeric_limits< float >::epsilon();
            _computeSplit( node->right, time - leftTime, datas, childVP, range);
            break;
        }

        case MODE_DB:
        {
            LBASSERT( vp == Viewport::FULL );
            float splitPos = range.start;
            const float end = range.end;

            while( timeLeft > std::numeric_limits< float >::epsilon() &&
                   splitPos < end )
            {
                LBLOG( LOG_LB2 ) << timeLeft << "ms left using "
                                 << workingSet.size() << " tiles" << std::endl;

                // remove all irrelevant items from working set
                for( LBDatas::iterator i = workingSet.begin();
                     i != workingSet.end(); )
                {
                    const Data& data = *i;
                    if( data.range.end > splitPos )
                        ++i;
                    else
                        i = workingSet.erase( i );
                }
                if( workingSet.empty( ))
                    break;

                // find next 'discontinouity' in loads
                float currentPos = 1.0f;
                for( LBDatas::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;
                    currentPos = LB_MIN( currentPos, data.range.end );
                }

                const float size = currentPos - splitPos;
                LBASSERTINFO( size > 0.f, currentPos << "<=" << splitPos );
                LBASSERT( currentPos <= 1.0f );

                // accumulate normalized load in splitPos...currentPos
                LBLOG( LOG_LB2 ) << "Computing load in range " << splitPos
                                << "..." << currentPos << std::endl;
                float currentTime = 0.f;
                for( LBDatas::const_iterator i = workingSet.begin();
                     i != workingSet.end(); ++i )
                {
                    const Data& data = *i;

                    if( data.range.start >= currentPos ) // not yet needed data
                        break;
#if 0
                    // make sure we cover full area
                    LBASSERTINFO(  data.range.start <= splitPos,
                                   data.range.start << " > " << splitPos );
                    LBASSERTINFO( data.range.end >= currentPos,
                                  data.range.end << " < " << currentPos);
#endif
                    currentTime += data.time * size / data.range.getSize();
                }

                LBLOG( LOG_LB2 ) << splitPos << "..." << currentPos << ": t="
                                 << currentTime << " of " << timeLeft
                                 << std::endl;

                if( currentTime >= timeLeft ) // found last region
                {
                    const float width = currentPos - splitPos;
                    splitPos += (width * timeLeft / currentTime );
                    timeLeft = 0.0f;
                }
                else
                {
                    timeLeft -= currentTime;
                    splitPos  = currentPos;
                }
            }
            LBLOG( LOG_LB2 ) << "Should split at " << splitPos << std::endl;
            if( getDamping() < 1.f )
                splitPos = (1.f - getDamping( )) * splitPos +
                            getDamping() * node->split;
            LBLOG( LOG_LB2 ) << "Dampened split at " << splitPos << std::endl;

            const float boundary( node->boundaryf );
            if( node->left->resources == 0.f )
                splitPos = range.start;
            else if( node->right->resources == 0.f )
                splitPos = end;

            const uint32_t ratio = static_cast< uint32_t >
                      ( splitPos / boundary + .5f );
            splitPos = ratio * boundary;
            if( (splitPos - range.start) < boundary )
                splitPos = range.start;
            if( (end - splitPos) < boundary )
                splitPos = end;

            if( fabs( splitPos - node->split ) < node->resistancef )
                splitPos = node->split;
            else
                node->split = splitPos;

            LBLOG( LOG_LB2 ) << "Constrained split " << range << " at pos "
                             << splitPos << std::endl;

            Range childRange = range;
            childRange.end = splitPos;
            _computeSplit( node->left, leftTime, datas, vp, childRange );

            childRange.start = childRange.end;
            childRange.end   = range.end;
            _computeSplit( node->right, time - leftTime, datas, vp, childRange);
            break;
        }

        default:
            LBUNIMPLEMENTED;
    }
}

void LoadEqualizer::_assign( Compound* compound, const Viewport& vp,
                             const Range& range )
{
    LBASSERTINFO( vp == Viewport::FULL || range == Range::ALL,
                  "Mixed 2D/DB load-balancing not implemented" );

    compound->setViewport( vp );
    compound->setRange( range );
    LBLOG( LOG_LB2 ) << compound->getChannel()->getName() << " set " << vp
                     << ", " << range << std::endl;

    if( getDamping() >= 1.f )
        return;

    // save data for later use
    Data data;
    data.vp      = vp;
    data.range   = range;
    data.channel = compound->getChannel();
    data.taskID  = compound->getTaskID();

    const Compound* destCompound = getCompound();
    if( destCompound->getChannel() == compound->getChannel( ))
        data.destTaskID  = destCompound->getTaskID();

    LBASSERT( data.taskID > 0 );

    if( !vp.hasArea() || !range.hasData( )) // will not render
        data.time = 0;

    LBFrameData& frameData = _history.back();
    LBDatas& items = frameData.second;

    items.push_back( data );
}

std::ostream& operator << ( std::ostream& os, const LoadEqualizer::Node* node )
{
    if( !node )
        return os;

    os << lunchbox::disableFlush;

    if( node->compound )
        os << node->compound->getChannel()->getName() << " resources "
           << node->resources << " max size " << node->maxSize << std::endl;
    else
        os << "split " << node->mode << " @ " << node->split << " resources "
           << node->resources << " max size " << node->maxSize  << std::endl
           << lunchbox::indent << node->left << node->right << lunchbox::exdent;

    os << lunchbox::enableFlush;
    return os;
}

std::ostream& operator << ( std::ostream& os, const LoadEqualizer* lb )
{
    if( !lb )
        return os;

    os << lunchbox::disableFlush
       << "load_equalizer" << std::endl
       << '{' << std::endl
       << "    mode    " << lb->getMode() << std::endl;

    if( lb->getDamping() != 0.5f )
        os << "    damping " << lb->getDamping() << std::endl;

    if( lb->getBoundary2i() != Vector2i( 1, 1 ) )
        os << "    boundary [ " << lb->getBoundary2i().x() << " "
           << lb->getBoundary2i().y() << " ]" << std::endl;

    if( lb->getBoundaryf() != std::numeric_limits<float>::epsilon() )
        os << "    boundary " << lb->getBoundaryf() << std::endl;

    if( lb->getResistance2i() != Vector2i( 0, 0 ) )
        os << "    resistance [ " << lb->getResistance2i().x() << " "
           << lb->getResistance2i().y() << " ]" << std::endl;

    if( lb->getResistancef() != .0f )
        os << "    resistance " << lb->getResistancef() << std::endl;

    os << '}' << std::endl << lunchbox::enableFlush;
    return os;
}

}
}
