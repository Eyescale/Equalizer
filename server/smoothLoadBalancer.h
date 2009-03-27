
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
    /** 
     * Adapts the frame rate of a compound to smoothen its output. 
     * 
     * Does not support period settings underneath a child. One channel should
     * not be used in compounds with a different inherit period.
     */
    class SmoothLoadBalancer : public LoadBalancerIF
    {
    public:
        SmoothLoadBalancer( const LoadBalancer& parent );
        virtual ~SmoothLoadBalancer();

        /** @sa LoadBalancerIF::update */
        virtual void update( const uint32_t frameNumber );

    private:
        /** Frame number with max time. */
        typedef std::pair< uint32_t, float > FrameTime;
        
        /** Historical data to compute new frame rate. */
        std::deque< FrameTime > _times;

        /** Helper class connecting on child tree for load gathering. */
        class LoadListener : public ChannelListener
        {
        public:
            /** @sa ChannelListener::notifyLoadData */
            virtual void notifyLoadData( Channel* channel, 
                                         const uint32_t frameNumber,
                                         const uint32_t nStatistics,
                                         const eq::Statistic* statistics  );

            SmoothLoadBalancer* parent;
            uint32_t            period;
        };

        /** One listener for each compound child. */
        std::vector< LoadListener > _loadListeners;
        friend class LoadListener;

        /** The number of samples to use from _times. */
        uint32_t _nSamples;

        /** Initialize for the current _parent. */
        bool _init();
    };
}
}

#endif // EQS_LOADBALANCER_H
