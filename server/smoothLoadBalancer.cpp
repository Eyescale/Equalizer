
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

    size_t nValues    = 0;
    float  totalTime  = 0.f;
    
    deque< FrameTime >::iterator i = _times.begin();
    for( ; i != _times.end() && nValues < _nSamples; ++i )
    {
        const FrameTime& time = *i;
        if( time.second == 0.f )
            continue;

        EQASSERT( time.first > 0 );
        ++nValues;
        totalTime += time.second;
    }

    if( nValues == _nSamples && i != _times.end( ))
        _times.erase( i, _times.end( ));

    if( nValues > 0 )
    {
        //TODO: totalTime *= 1.f - damping;
        const float fps = 1000.f / totalTime * nValues;
        EQLOG( LOG_LB ) << fps << " Hz from " << nValues << " samples, "
                        << totalTime << "ms" << endl;
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
