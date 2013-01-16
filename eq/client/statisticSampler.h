
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_STATISTICSAMPLER_H
#define EQ_STATISTICSAMPLER_H

#include <eq/client/event.h> // member

namespace eq
{
    /**
     * Utility to sample an statistics event.
     *
     * Holds a ConfigEvent, which is initialized from the owner's data during
     * initialization. Subclasses implement the constructor and destructor to
     * sample the times and process the gathered statistics.
     */
    template< typename Owner > class StatisticSampler
    {
    public:
        /**
         * Construct a new statistics sampler.
         *
         * @param type The statistics type.
         * @param owner The originator of the statistics event.
         * @param frameNumber The current frame.
         * @version 1.0
         */
        StatisticSampler( const Statistic::Type type, Owner* owner,
                          const uint32_t frameNumber = LB_UNDEFINED_UINT32 )
            : _owner( owner )
        {
            LBASSERT( owner );
            LBASSERT( owner->getID() != 0 );
            LBASSERT( owner->getSerial() != EQ_INSTANCE_INVALID );
            event.type                  = Event::STATISTIC;
            event.serial                = owner->getSerial();
            event.originator            = owner->getID();
            event.statistic.type        = type;
            event.statistic.frameNumber = frameNumber;
            event.statistic.resourceName[0] = '\0';
            event.statistic.startTime   = 0;
            event.statistic.endTime     = 0;

            if( event.statistic.frameNumber == LB_UNDEFINED_UINT32 )
                event.statistic.frameNumber = owner->getCurrentFrame();
        }

        /** Destruct and finish statistics sampling. @version 1.0 */
        virtual ~StatisticSampler()
        {
            LBASSERTINFO( event.statistic.startTime <= event.statistic.endTime,
                          event.statistic );
        }

        Event event; //!< The statistics event.

    protected:
        Owner* const _owner;
    };

}

#endif // EQ_STATISTICSAMPLER_H
