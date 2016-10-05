
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/types.h>
#include <eq/fabric/statistic.h> // member

namespace eq
{
/**
 * Utility to sample an statistics event.
 *
 * Holds a Statistic, which is initialized from the owner's data in the
 * ctor. Subclasses implement the constructor and destructor to sample the times
 * and process the gathered statistics.
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
        LBASSERT( owner->getSerial() != CO_INSTANCE_INVALID );
        statistic.type = type;
        statistic.frameNumber = frameNumber;
        statistic.resourceName[0] = '\0';
        statistic.startTime = 0;
        statistic.endTime = 0;

        if( statistic.frameNumber == LB_UNDEFINED_UINT32 )
            statistic.frameNumber = owner->getCurrentFrame();
    }

    /** Destruct and finish statistics sampling. @version 1.0 */
    virtual ~StatisticSampler()
    {
        LBASSERTINFO( statistic.startTime <= statistic.endTime, statistic );
    }

    Statistic statistic; //!< The statistics event.

protected:
    Owner* const _owner;
};

}

#endif // EQ_STATISTICSAMPLER_H
