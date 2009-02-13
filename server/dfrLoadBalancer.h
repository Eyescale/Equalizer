
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_DFRLOADBALANCER_H
#define EQS_DFRLOADBALANCER_H

#include "channelListener.h" // base class
#include "loadBalancer.h"    // base class LoadBalancerIF

#include <deque>
#include <map>

namespace eq
{
namespace server
{
    /** 
     * Adapts the frame rate averarage while adapt a zoom factor. 
     * 
     */
    class DfrLoadBalancer : public LoadBalancerIF, protected ChannelListener
    {
    public:

        DfrLoadBalancer( const LoadBalancer& parent );
        virtual ~DfrLoadBalancer();

        /** @sa LoadBalancerIF::update */
        virtual void update( const uint32_t frameNumber );
        
        
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const uint32_t nStatistics,
                                     const eq::Statistic* statistics  );
    private:
        Compound* _compound;

        float _fpsLastFrame;

    };
}
}

#endif // EQS_DFRLOADBALANCER_H
