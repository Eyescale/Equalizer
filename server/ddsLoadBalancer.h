
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
