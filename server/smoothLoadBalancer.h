
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SMOOTHLOADBALANCER_H
#define EQS_SMOOTHLOADBALANCER_H

#include "channelListener.h" // base class
#include "loadBalancer.h"    // base class LoadBalancerIF

#include <deque>
#include <map>

namespace eq
{
namespace server
{
    /** Adapts the frame rate of a compound to smoothen its output. */
    class SmoothLoadBalancer : public LoadBalancerIF, protected ChannelListener
    {
    public:
        SmoothLoadBalancer( const LoadBalancer& parent );
        virtual ~SmoothLoadBalancer();

        /** @sa LoadBalancerIF::update */
        virtual void update( const uint32_t frameNumber );

        /** @sa ChannelListener::notifyLoadData */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const float startTime, const float endTime
                                     /*, const float load */ );

    private:
        /** Frame number with max time. */
        typedef std::pair< uint32_t, float > FrameTime;
        
        /** Historical data to compute new frame rate. */
        std::deque< FrameTime > _times;

        /** Used to identify the compound for a channel. */
        std::map< Channel*, Compound* > _compoundMap;

        /** The number of samples to use from _times. */
        uint32_t _nSamples;

        /** Initialize for the current _parent. */
        bool _init();
    };
}
}

#endif // EQS_LOADBALANCER_H
