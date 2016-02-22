
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQS_FRAMERATEEQUALIZER_H
#define EQS_FRAMERATEEQUALIZER_H

#include "../channelListener.h" // base class
#include "equalizer.h"          // base class

#include <deque>
#include <map>

namespace eq
{
namespace server
{
    std::ostream& operator << ( std::ostream& os, const FramerateEqualizer* );

    /**
     * Adapts the frame rate of a compound to smoothen its output.
     *
     * Does not support period settings underneath a child. One channel should
     * not be used in compounds with a different inherit period.
     */
    class FramerateEqualizer : public Equalizer
    {
    public:
        EQSERVER_API FramerateEqualizer();
        FramerateEqualizer( const FramerateEqualizer& from );
        virtual ~FramerateEqualizer();
        void toStream( std::ostream& os ) const final { os << this; }

        /** @sa Equalizer::attach */
        void attach( Compound* compound ) final;

        /** @sa CompoundListener::notifyUpdatePre */
        void notifyUpdatePre( Compound* compound,
                              const uint32_t frameNumber ) final;

        uint32_t getType() const final { return fabric::FRAMERATE_EQUALIZER; }

    protected:
        void notifyChildAdded( Compound*, Compound* ) override
            { LBASSERT( _nSamples == 0 ); }
        void notifyChildRemove( Compound*, Compound* ) override
            { LBASSERT( _nSamples == 0 ); }

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
                                         uint32_t frameNumber,
                                         const Statistics& statistics,
                                         const Viewport& region );

            FramerateEqualizer* parent;
            uint32_t period;
        };

        /** One listener for each compound child. */
        std::vector< LoadListener > _loadListeners;
        friend class LoadListener;

        /** The number of samples to use from _times. */
        uint32_t _nSamples;

        void _init();
        void _exit();
    };
}
}

#endif // EQS_LOADBALANCER_H
