
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 All rights reserved. */

#ifndef EQS_DDSLOADBALANCER_H
#define EQS_DDSLOADBALANCER_H

#include "channelListener.h" // base class
#include "loadBalancer.h"    // base class LoadBalancerIF
#include "eq/client/types.h"
#include <deque>
#include <map>

namespace eq
{
    namespace server
    {
        class Compound;
        
        /** Destination-driven scaling.*/
        class DDSLoadBalancer : public LoadBalancerIF, protected ChannelListener
        {
        public:
            
            DDSLoadBalancer( const LoadBalancer& parent );
            virtual ~DDSLoadBalancer();
            
            /** @sa LoadBalancerIF::update */
            virtual void update( const uint32_t frameNumber );
            
            /** @sa ChannelListener::notifyLoadData */
            virtual void notifyLoadData( Channel* channel, 
                                        const uint32_t frameNumber,
                                        const uint32_t nStatistics,
                                        const eq::Statistic* statistics  );
        private:
            /** compute destination size, input frame offset and 
                output frame zoom value */
            void _updateZoomAndOffset();
            
            Compound* const _compound;
            
            eq::ViewportVector _viewports;
            
            FrameVector _outputFrames;
        };
    }
}

#endif // EQS_DDSLOADBALANCER_H
