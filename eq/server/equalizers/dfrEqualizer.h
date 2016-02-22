
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQS_DFREQUALIZER_H
#define EQS_DFREQUALIZER_H

#include "../channelListener.h" // base class
#include "equalizer.h"       // base class

#include <deque>
#include <map>

namespace eq
{
namespace server
{
    std::ostream& operator << ( std::ostream& os, const DFREqualizer* );

    /** Tries to maintain a constant frame rate by adapting the compound zoom.*/
    class DFREqualizer : public Equalizer, protected ChannelListener
    {
    public:
        DFREqualizer();
        virtual ~DFREqualizer();
        void toStream( std::ostream& os ) const final { os << this; }

        /** @sa Equalizer::attach */
        void attach( Compound* compound ) final;

        /** @sa CompoundListener::notifyUpdatePre */
        void notifyUpdatePre( Compound* compound,
                              const uint32_t frameNumber ) final;

        /** @sa ChannelListener::notifyLoadData */
        void notifyLoadData( Channel* channel,
                             uint32_t frameNumber,
                             const Statistics& statistics,
                             const Viewport& region ) final;

        uint32_t getType() const final { return fabric::DFR_EQUALIZER; }

    protected:
        void notifyChildAdded( Compound*, Compound* ) override {}
        void notifyChildRemove( Compound*, Compound* ) override {}

    private:
        float _current; //!< Framerate of the last finished frame
        int64_t _lastTime; //!< Last frames' timestamp
    };

}
}

#endif // EQS_DFREQUALIZER_H
