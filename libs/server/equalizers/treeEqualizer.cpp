
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include "treeEqualizer.h"

#include "../compound.h"
#include "../log.h"

#include <eq/statistic.h>
#include <co/base/debug.h>

namespace eq
{
namespace server
{

std::ostream& operator << ( std::ostream& os, const TreeEqualizer::Node* );

// The tree load balancer organizes the children in a binary tree. At each
// level, a relative split position is determined by balancing the left subtree
// against the right subtree.

TreeEqualizer::TreeEqualizer()
        : _mode( MODE_2D )
        , _damping( .5f )
        , _boundary2i( 1, 1 )
        , _boundaryf( std::numeric_limits<float>::epsilon() )
        , _tree( 0 )

{
    EQINFO << "New TreeEqualizer @" << (void*)this << std::endl;
}

TreeEqualizer::TreeEqualizer( const TreeEqualizer& from )
        : Equalizer( from )
        , ChannelListener( from )
        , _mode( from._mode )
        , _damping( from._damping )
        , _boundary2i( from._boundary2i )
        , _boundaryf( from._boundaryf )
        , _tree( 0 )
{}

TreeEqualizer::~TreeEqualizer()
{
    _clearTree( _tree );
    delete _tree;
    _tree = 0;
}

void TreeEqualizer::notifyUpdatePre( Compound* compound,
                                     const uint32_t frameNumber )
{
    if( isFrozen() || !compound->isRunning( ))
        return;

    if( !_tree )
    {
        EQASSERT( compound == getCompound( ));
        const Compounds& children = compound->getChildren();
        if( children.empty( )) // leaf compound, can't do anything.
            return;

        _tree = _buildTree( children );
        _init( _tree );
    }

    // compute new data
    _update( _tree );
    _split( _tree );
    _assign( _tree, Viewport(), Range( ));
    EQLOG( LOG_LB2 ) << "LB tree: " << _tree;
}

TreeEqualizer::Node* TreeEqualizer::_buildTree( const Compounds& compounds )
{
    Node* node = new Node;

    const size_t size = compounds.size();
    if( size == 1 )
    {
        Compound* compound = compounds.front();

        node->compound  = compound;
        node->mode = _mode;

        Channel* channel = compound->getChannel();
        EQASSERT( channel );
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
    node->mode = _mode;

    return node;
}

void TreeEqualizer::_init( Node* node )
{
    if( node->compound )
    {
        EQASSERT( node->mode != MODE_2D );
        return;
    }
    // else

    if( node->mode == MODE_2D )
        node->mode = MODE_VERTICAL;

    if( node->left->mode == MODE_2D )
    {
        EQASSERT( node->right->mode == MODE_2D );

        node->left->mode = (node->mode == MODE_VERTICAL) ? MODE_HORIZONTAL : 
                                                           MODE_VERTICAL;
        node->right->mode = node->left->mode;
    }
    _init( node->left );
    _init( node->right );
}


void TreeEqualizer::_clearTree( Node* node )
{
    if( !node )
        return;

    if( node->compound )
    {
        Channel* channel = node->compound->getChannel();
        EQASSERTINFO( channel, node->compound );
        channel->removeListener( this );
    }
    else
    {
        _clearTree( node->left );
        _clearTree( node->right );
    }
}

void TreeEqualizer::notifyLoadData( Channel* channel,
                                    const uint32_t frameNumber,
                                    const uint32_t nStatistics,
                                    const Statistic* statistics )
{
    _notifyLoadData( _tree, channel, nStatistics, statistics );
}

void TreeEqualizer::_notifyLoadData( Node* node, Channel* channel,
                                     const uint32_t nStatistics,
                                     const Statistic* statistics )
{
    if( !node )
        return;

    _notifyLoadData( node->left, channel, nStatistics, statistics );
    _notifyLoadData( node->right, channel, nStatistics, statistics );

    if( !node->compound || node->compound->getChannel() != channel )
        return;

    // gather relevant load data
    const uint32_t taskID = node->compound->getTaskID();
    int64_t startTime = std::numeric_limits< int64_t >::max();
    int64_t endTime   = 0;
    bool    loadSet   = false;
    int64_t timeTransmit = 0;
    for( uint32_t i = 0; i < nStatistics && !loadSet; ++i )
    {
        const Statistic& stat = statistics[ i ];
        if( stat.task != taskID ) // from different compound
            continue;

        switch( stat.type )
        {
        case Statistic::CHANNEL_CLEAR:
        case Statistic::CHANNEL_DRAW:
        case Statistic::CHANNEL_READBACK:
            startTime = EQ_MIN( startTime, stat.startTime );
            endTime   = EQ_MAX( endTime, stat.endTime );
            break;

        case Statistic::CHANNEL_FRAME_TRANSMIT:
            timeTransmit += stat.endTime - stat.startTime;
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

    node->time = endTime - startTime;
    node->time = EQ_MAX( node->time, 1 );
    node->time = EQ_MAX( node->time, timeTransmit );
}

void TreeEqualizer::_update( Node* node )
{
    if( !node )
        return;

    const Compound* compound = node->compound;
    if( compound )
    {
        const Channel* channel = compound->getChannel();
        const PixelViewport& pvp = channel->getPixelViewport();
        EQASSERT( channel );

        node->resources = compound->isRunning() ? compound->getUsage() : 0.f;
        node->maxSize.x() = pvp.w; 
        node->maxSize.y() = pvp.h; 
        node->boundaryf = _boundaryf;
        node->boundary2i = _boundary2i;
        return;
    }
    // else

    EQASSERT( node->left );
    EQASSERT( node->right );

    _update( node->left );
    _update( node->right );

    node->resources = node->left->resources + node->right->resources;

    if( node->left->resources == 0.f )
    {
        node->maxSize = node->right->maxSize;
        node->boundary2i = node->right->boundary2i;
        node->boundaryf = node->right->boundaryf;
        node->time = node->right->time;
    }
    else if( node->right->resources == 0.f )
    {
        node->maxSize = node->left->maxSize;
        node->boundary2i = node->left->boundary2i;
        node->boundaryf = node->left->boundaryf;
        node->time = node->left->time;
    }
    else
    {
        switch( node->mode )
        {
        case MODE_VERTICAL:
            node->maxSize.x() = node->left->maxSize.x() +
                                node->right->maxSize.x();  
            node->maxSize.y() = EQ_MIN( node->left->maxSize.y(), 
                                        node->right->maxSize.y() ); 
            node->boundary2i.x() = node->left->boundary2i.x() +
                                   node->right->boundary2i.x();
            node->boundary2i.y() = EQ_MAX( node->left->boundary2i.y(), 
                                           node->right->boundary2i.y());
            node->boundaryf = EQ_MAX( node->left->boundaryf,
                                      node->right->boundaryf );
            break;
        case MODE_HORIZONTAL:
            node->maxSize.x() = EQ_MIN( node->left->maxSize.x(), 
                                        node->right->maxSize.x() );  
            node->maxSize.y() = node->left->maxSize.y() +
                                node->right->maxSize.y(); 
            node->boundary2i.x() = EQ_MAX( node->left->boundary2i.x(), 
                                           node->right->boundary2i.x() );
            node->boundary2i.y() = node->left->boundary2i.y() +
                                   node->right->boundary2i.y();
            node->boundaryf = EQ_MAX( node->left->boundaryf,
                                      node->right->boundaryf );
            break;
        case MODE_DB:
            node->boundary2i.x() = EQ_MAX( node->left->boundary2i.x(), 
                                           node->right->boundary2i.x() );
            node->boundary2i.y() = EQ_MAX( node->left->boundary2i.y(), 
                                           node->right->boundary2i.y() );
            node->boundaryf = node->left->boundaryf +node->right->boundaryf;
            break;
        default:
            EQUNIMPLEMENTED;
        }

        node->time = node->left->time + node->right->time;
    }
}

void TreeEqualizer::_split( Node* node )
{
    if( node->compound )
        return;
    EQASSERT( node->left && node->right );

    Node* left = node->left;
    Node* right = node->right;
    // easy outs
    if( left->resources == 0.f )
    {
        node->split = 0.f;
        return;
    }
    if( right->resources == 0.f )
    {
        node->split = 1.f;
        return;
    }

    // new split
    const float target = node->time * left->resources / node->resources;
    const float leftTime = float(left->time);
    float split = 0.f;
    const float rightTime = float(right->time);

    if( leftTime >= target )
        split = target / leftTime * node->split;
    else
    {
        const float timeLeft = target - leftTime;
        split = node->split + timeLeft / rightTime * ( 1.f - node->split );
    }

    EQLOG( LOG_LB2 )
        << "Should split at " << split << " (" << target << ": " << leftTime
        << " by " << left->resources << "/" << rightTime << " by "
        << right->resources << ")" << std::endl;
    node->split = (1.f - _damping) * split + _damping * node->split;
    EQLOG( LOG_LB2 ) << "Dampened split at " << node->split << std::endl;

    _split( left );
    _split( right );
}

void TreeEqualizer::_assign( Node* node, const Viewport& vp,
                             const Range& range )
{
    EQLOG( LOG_LB2 ) << "assign " << vp << ", " << range << " time "
                     << node->time << " split " << node->split << std::endl;
    EQASSERTINFO( vp.isValid(), vp );
    EQASSERTINFO( range.isValid(), range );
    EQASSERTINFO( node->resources > 0.f || !vp.hasArea() || !range.hasData(),
                  "Assigning work to unused compound: " << vp << ", " << range);

    Compound* compound = node->compound;
    if( compound )
    {
        EQASSERTINFO( vp == Viewport::FULL || range == Range::ALL,
                      "Mixed 2D/DB load-balancing not implemented" );

        compound->setViewport( vp );
        compound->setRange( range );
        EQLOG( LOG_LB2 ) << compound->getChannel()->getName() << " set " << vp
                         << ", " << range << std::endl;
        return;
    }

    switch( node->mode )
    {
    case MODE_VERTICAL:
    {
        // Ensure minimum size
        const Compound* root = getCompound();
        const float pvpW = float( root->getInheritPixelViewport().w );
        const float end = vp.getXEnd();
        const float boundary = float( node->boundary2i.x( )) / pvpW;
        float absoluteSplit = vp.x + vp.w * node->split;

        if( node->left->resources == 0.f )
            absoluteSplit = vp.x;
        else if( node->right->resources == 0.f )
            absoluteSplit = end;
        else if( boundary > 0 )
        {
            const float right = vp.getXEnd() - absoluteSplit;
            const float left = absoluteSplit - vp.x;
            const float maxRight = float( node->right->maxSize.x( )) / pvpW;
            const float maxLeft = float( node->left->maxSize.x( )) / pvpW;

            if( right > maxRight )
                absoluteSplit = end - maxRight;
            else if( left > maxLeft )
                absoluteSplit = vp.x + maxLeft;
            
            if( (absoluteSplit - vp.x) < boundary )
                absoluteSplit = vp.x + boundary;
            if( (end - absoluteSplit) < boundary )
                absoluteSplit = end - boundary;
                
            const uint32_t ratio = uint32_t( absoluteSplit / boundary + .5f );
            absoluteSplit = ratio * boundary;
        }

        absoluteSplit = EQ_MAX( absoluteSplit, vp.x );
        absoluteSplit = EQ_MIN( absoluteSplit, end);

        node->split = (absoluteSplit - vp.x ) / vp.w;
        EQLOG( LOG_LB2 ) << "Constrained split " << vp << " at X "
                         << node->split << std::endl;

        // traverse children
        Viewport childVP = vp;
        childVP.w = (absoluteSplit - vp.x);
        _assign( node->left, childVP, range );

        childVP.x = childVP.getXEnd();
        childVP.w = end - childVP.x;

        // Fix 2994111: Rounding errors with 2D LB and 16 sources
        //   Floating point rounding may create a width for the 'right'
        //   child which is slightly below the parent width. Correct it.
        while( childVP.getXEnd() < end )
            childVP.w += std::numeric_limits< float >::epsilon();

        _assign( node->right, childVP, range );
        break;
    }

    case MODE_HORIZONTAL:
    {
        // Ensure minimum size
        const Compound* root = getCompound();
        const float pvpH = float( root->getInheritPixelViewport().h );
        const float end = vp.getYEnd();
        const float boundary = float( node->boundary2i.y( )) / pvpH;
        float absoluteSplit = vp.y + vp.h * node->split;

        if( node->left->resources == 0.f )
            absoluteSplit = vp.y;
        else if( node->right->resources == 0.f )
            absoluteSplit = end;
        else if( boundary > 0 )
        {
            const float right = vp.getYEnd() - absoluteSplit;
            const float left = absoluteSplit - vp.y;
            const float maxRight = float( node->right->maxSize.y( )) / pvpH;
            const float maxLeft = float( node->left->maxSize.y( )) / pvpH;

            if( right > maxRight )
                absoluteSplit = end - maxRight;
            else if( left > maxLeft )
                absoluteSplit = vp.y + maxLeft;
            
            if( (absoluteSplit - vp.y) < boundary )
                absoluteSplit = vp.y + boundary;
            if( (end - absoluteSplit) < boundary )
                absoluteSplit = end - boundary;
                
            const uint32_t ratio = uint32_t( absoluteSplit / boundary + .5f );
            absoluteSplit = ratio * boundary;
        }

        absoluteSplit = EQ_MAX( absoluteSplit, vp.y );
        absoluteSplit = EQ_MIN( absoluteSplit, end);

        node->split = (absoluteSplit - vp.y ) / vp.h;
        EQLOG( LOG_LB2 ) << "Constrained split " << vp << " at X "
                         << node->split << std::endl;

        // traverse children
        Viewport childVP = vp;
        childVP.h = (absoluteSplit - vp.y);
        _assign( node->left, childVP, range );

        childVP.y = childVP.getYEnd();
        childVP.h = end - childVP.y;

        // Fix 2994111: Rounding errors with 2D LB and 16 sources
        //   Floating point rounding may create a width for the 'right'
        //   child which is slightly below the parent width. Correct it.
        while( childVP.getYEnd() < end )
            childVP.h += std::numeric_limits< float >::epsilon();

        _assign( node->right, childVP, range );
        break;
    }

    case MODE_DB:
    {
        EQASSERT( vp == Viewport::FULL );
        const float end = range.end;
        float absoluteSplit = range.start + (range.end-range.start)*node->split;

        const float boundary( node->boundaryf );
        if( node->left->resources == 0.f )
            absoluteSplit = range.start;
        else if( node->right->resources == 0.f )
            absoluteSplit = end;

        const uint32_t ratio = uint32_t( absoluteSplit / boundary + .5f );
        absoluteSplit = ratio * boundary;
        if( (absoluteSplit - range.start) < boundary )
            absoluteSplit = range.start;
        if( (end - absoluteSplit) < boundary )
            absoluteSplit = end;

        node->split = (absoluteSplit-range.start) / (range.end-range.start);
        EQLOG( LOG_LB2 ) << "Constrained split " << range << " at pos "
                         << node->split << std::endl;

        Range childRange = range;
        childRange.end = absoluteSplit;
        _assign( node->left, vp, childRange );

        childRange.start = childRange.end;
        childRange.end   = range.end;
        _assign( node->right, vp, childRange);
        break;
    }

    default:
        EQUNIMPLEMENTED;
    }
}

std::ostream& operator << ( std::ostream& os, const TreeEqualizer::Node* node )
{
    if( !node )
        return os;

    os << co::base::disableFlush;

    if( node->compound )
        os << node->compound->getChannel()->getName() << " resources " 
           << node->resources << " max size " << node->maxSize << std::endl;
    else
        os << "split " << node->mode << " @ " << node->split << " resources "
           << node->resources << " max size " << node->maxSize  << std::endl
           << co::base::indent << node->left << node->right << co::base::exdent;

    os << co::base::enableFlush;
    return os;
}

std::ostream& operator << ( std::ostream& os, 
                            const TreeEqualizer::Mode mode )
{
    os << ( mode == TreeEqualizer::MODE_2D         ? "2D" :
            mode == TreeEqualizer::MODE_VERTICAL   ? "VERTICAL" :
            mode == TreeEqualizer::MODE_HORIZONTAL ? "HORIZONTAL" :
            mode == TreeEqualizer::MODE_DB         ? "DB" : "ERROR" );
    return os;
}

std::ostream& operator << ( std::ostream& os, const TreeEqualizer* lb )
{
    if( !lb )
        return os;

    os << co::base::disableFlush
       << "tree_equalizer" << std::endl
       << '{' << std::endl
       << "    mode    " << lb->getMode() << std::endl;
  
    if( lb->getDamping() != 0.5f )
        os << "    damping " << lb->getDamping() << std::endl;

    if( lb->getBoundary2i() != Vector2i( 1, 1 ) )
        os << "    boundary [ " << lb->getBoundary2i().x() << " " 
           << lb->getBoundary2i().y() << " ]" << std::endl;

    if( lb->getBoundaryf() != std::numeric_limits<float>::epsilon() )
        os << "    boundary " << lb->getBoundaryf() << std::endl;

    os << '}' << std::endl << co::base::enableFlush;
    return os;
}

}
}
