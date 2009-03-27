
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "smoothLoadBalancer.h"

#include "compound.h"
#include "compoundVisitor.h"
#include "config.h"
#include "log.h"

#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

namespace
{
class LoadSubscriber : public CompoundVisitor
{
public:
    LoadSubscriber( ChannelListener* listener ) : _listener( listener ) {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel*  channel = compound->getChannel();
            EQASSERT( channel );
            channel->addListener( _listener );

            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
};

class LoadUnsubscriber : public CompoundVisitor
{
public:
    LoadUnsubscriber( ChannelListener* listener ) : _listener( listener ) {}

    virtual VisitorResult visit( Compound* compound )
        {
            Channel*  channel = compound->getChannel();
            EQASSERT( channel );
            channel->removeListener( _listener );

            return TRAVERSE_CONTINUE; 
        }

private:
    ChannelListener* const _listener;
};

}

// The smooth load balancer adapts the framerate of the compound to be the
// average frame rate of all children, taking the DPlex period into account.

SmoothLoadBalancer::SmoothLoadBalancer( const LoadBalancer& parent )
        : LoadBalancerIF( parent )
        , _nSamples( 0 )
{
    EQINFO << "New SmoothLoadBalancer @" << (void*)this << endl;
}

SmoothLoadBalancer::~SmoothLoadBalancer()
{
    const Compound*       compound = _parent.getCompound();
    const CompoundVector& children = compound->getChildren();

    EQASSERT( _loadListeners.size() == children.size( ));
    for( size_t i = 0; i < children.size(); ++i )
    {
        Compound*      child        = children[i];
        LoadListener&  loadListener = _loadListeners[i];

        LoadUnsubscriber unsubscriber( &loadListener );
        child->accept( unsubscriber );
    }

    _loadListeners.clear();
    _times.clear();
}

void SmoothLoadBalancer::update( const uint32_t frameNumber )
{
    Compound* compound = _parent.getCompound();

    if( _parent.isFrozen( ))
    {
        compound->setMaxFPS( numeric_limits< float >::max( ));
        return;
    }

    if( _nSamples == 0 )
        _init();

    // find starting point of contiguous block
    const size_t size = _times.size();
    size_t       from = 0;
    for( size_t i = 0; i < size; ++i )
    {
        if( _times[i].second == 0.f )
            from = i;
    }

    // find max time in block
    const Config*  config        = compound->getConfig();
    const uint32_t finishedFrame = config->getFinishedFrame();

    size_t nSamples = 0;
    float  maxTime  = 0.f;
    for( ++from; from < size && nSamples < _nSamples; ++from )
    {
        const FrameTime& time = _times[from];
        if( time.second == 0.f || time.first < finishedFrame )
            continue;

        EQASSERT( time.first > 0 );
        ++nSamples;
        maxTime = EQ_MAX( maxTime, time.second );
        EQLOG( LOG_LB ) << "Using " << time.first << ", " << time.second << "ms"
                        << endl;
    }

    if( nSamples == _nSamples )       // If we have a full set
        while( from < _times.size( )) //  delete all older samples
            _times.pop_back();

    if( nSamples > 0 )
    {
        //TODO: totalTime *= 1.f - damping;
        const float fps = 1000.f / maxTime;
        EQLOG( LOG_LB ) << fps << " Hz from " << nSamples << " samples, "
                        << maxTime << "ms" << endl;
        compound->setMaxFPS( fps );
    }

    _times.push_front( FrameTime( frameNumber, 0.f ));
    EQASSERT( _times.size() < 210 );
}

bool SmoothLoadBalancer::_init()
{
    _nSamples = 1;

    // Subscribe to child channel load events
    const Compound*       compound = _parent.getCompound();
    const CompoundVector& children = compound->getChildren();
    
    EQASSERT( _loadListeners.empty( ));
    _loadListeners.resize( children.size( ));

    for( size_t i = 0; i < children.size(); ++i )
    {
        Compound*      child        = children[i];
        const uint32_t period       = child->getInheritPeriod();
        LoadListener&  loadListener = _loadListeners[i];

        loadListener.parent = this;
        loadListener.period = period;

        LoadSubscriber subscriber( &loadListener );
        child->accept( subscriber );

        _nSamples = EQ_MAX( _nSamples, period );
    }

    _nSamples = EQ_MIN( _nSamples, 100 );       
    return true;
}

void SmoothLoadBalancer::LoadListener::notifyLoadData( 
                            Channel* channel,
                            const uint32_t frameNumber, 
                            const uint32_t nStatistics,
                            const eq::Statistic* statistics  )
{
    // gather and notify load data
    float startTime = numeric_limits< float >::max();
    float endTime   = 0.0f;
    for( uint32_t i = 0; i < nStatistics; ++i )
    {
        const eq::Statistic& data = statistics[i];
        switch( data.type )
        {
            case eq::Statistic::CHANNEL_CLEAR:
            case eq::Statistic::CHANNEL_DRAW:
            case eq::Statistic::CHANNEL_ASSEMBLE:
            case eq::Statistic::CHANNEL_READBACK:
#ifndef EQ_ASYNC_TRANSMIT
            case eq::Statistic::CHANNEL_TRANSMIT:
#endif
                startTime = EQ_MIN( startTime, data.startTime );
                endTime   = EQ_MAX( endTime, data.endTime );
                break;
                
            default:
                break;
        }
    }
    
    if( startTime == numeric_limits< float >::max( ))
        return;
    
    for( deque< FrameTime >::iterator i = parent->_times.begin();
         i != parent->_times.end(); ++i )
    {
        FrameTime& frameTime = *i;
        if( frameTime.first != frameNumber )
            continue;

        const float time = (endTime - startTime) / period;
        frameTime.second = EQ_MAX( frameTime.second, time );
        EQLOG( LOG_LB ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << " period " << period << endl;
    }
}

}
}
