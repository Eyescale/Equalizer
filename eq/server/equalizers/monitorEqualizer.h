
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

#ifndef EQS_MONITOREQUALIZER_H
#define EQS_MONITOREQUALIZER_H

#include "equalizer.h"          // base class

#include <eq/client/types.h>
#include <deque>
#include <map>

namespace eq
{
namespace server
{
    std::ostream& operator << ( std::ostream& os, const MonitorEqualizer* );

    /** Destination-driven scaling.*/
    class MonitorEqualizer : public Equalizer
    {
    public:
        MonitorEqualizer();
        MonitorEqualizer( const MonitorEqualizer& from );
        virtual ~MonitorEqualizer();
        virtual void toStream( std::ostream& os ) const { os << this; }

        /** @sa Equalizer::attach. */
        virtual void attach( Compound* compound );

        /** @sa CompoundListener::notifyUpdatePre */
        virtual void notifyUpdatePre( Compound* compound,
                                      const uint32_t frameNumber );

        virtual uint32_t getType() const { return fabric::MONITOR_EQUALIZER; }

    protected:
        void notifyChildAdded( Compound*, Compound* ) override {}
        void notifyChildRemove( Compound*, Compound* ) override {}

    private:
        /** Init the source frame viewports. */
        void _updateViewports();

        /** compute destination size, input frame offset and
            output frame zoom value */
        void _updateZoomAndOffset();

        eq::Viewports _viewports;
        Frames _outputFrames;
    };
}
}

#endif // EQS_MONITOREQUALIZER_H
