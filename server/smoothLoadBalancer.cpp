
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#define NOMINMAX
#include "smoothLoadBalancer.h"

#include "compound.h"
#include "log.h"

#include <eq/base/debug.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace server
{

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

    for( CompoundVector::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        Compound* child   = *i;
        Channel*  channel = child->getChannel();
        EQASSERT( channel );
        channel->removeListener( this );
    }

    _times.clear();
    _compoundMap.clear();
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
    size_t nSamples = 0;
    float  maxTime  = 0.f;
    for( ++from; from < size && nSamples < _nSamples; ++from )
    {
        const FrameTime& time = _times[from];
        if( time.second == 0.f )
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

    for( CompoundVector::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        Compound* child   = *i;
        Channel*  channel = child->getChannel();
        EQASSERT( channel );

        EQASSERTINFO( _compoundMap.find( channel ) == _compoundMap.end(),
                      "Unimplemented feature: Usage of the same channel '"
                      << channel->getName() << "' multiple compound children"
                      << endl );

        _compoundMap[ channel ] = child;
        channel->addListener( this );
        _nSamples = EQ_MAX( _nSamples, child->getInheritPeriod( ));
    }

    _nSamples = EQ_MIN( _nSamples, 100 );       
    return true;
}

void SmoothLoadBalancer::notifyLoadData( Channel* channel, 
                                         const uint32_t frameNumber,
                                         const float startTime, 
                                         const float endTime
                                         /*, const float load */ )
{
    for( deque< FrameTime >::iterator i = _times.begin();
         i != _times.end(); ++i )
    {
        FrameTime& frameTime = *i;
        if( frameTime.first != frameNumber )
            continue;

        EQASSERT( _compoundMap.find( channel ) != _compoundMap.end( ));
        
        const Compound* compound = _compoundMap[ channel ];
        const float     period   = static_cast< float >( 
                                       compound->getInheritPeriod( ));
        const float     time     = (endTime - startTime) / period;

        frameTime.second = EQ_MAX( frameTime.second, time );
        EQLOG( LOG_LB ) << "Frame " << frameNumber << " channel " 
                        << channel->getName() << " time " << time
                        << " period " << period << endl;
    }
}

}
}
