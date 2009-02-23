
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
    /** Tries to maintain a constant frame rate by adapting the compound zoom.*/
    class DFRLoadBalancer : public LoadBalancerIF, protected ChannelListener
    {
    public:

        DFRLoadBalancer( const LoadBalancer& parent );
        virtual ~DFRLoadBalancer();

        /** @sa LoadBalancerIF::update */
        virtual void update( const uint32_t frameNumber );
        
        /** @sa ChannelListener::notifyLoadData */
        virtual void notifyLoadData( Channel* channel, 
                                     const uint32_t frameNumber,
                                     const uint32_t nStatistics,
                                     const eq::Statistic* statistics  );
    private:
        Compound*  const  _compound;
        float _fpsLastFrame;
        float _average;    
        int _sizeAverage;
        bool _newValueReady;
                
        int _count ;


    };
}
}

#endif // EQS_DFRLOADBALANCER_H
