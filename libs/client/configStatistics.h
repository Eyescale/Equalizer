
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CONFIGSTATISTICS_H
#define EQ_CONFIGSTATISTICS_H

#include <eq/statisticSampler.h> // base class

namespace eq
{
    class Config;

    /** Samples one Config statistics event. */
    class ConfigStatistics : public StatisticSampler< Config >
    {
    public:
        /**
         * Construct a statistics sampler and sample the start time.
         * @version 1.0
         */
        ConfigStatistics( const Statistic::Type type, Config* config );

        /**
         * Destruct the sampler, sample the end time and send the event.
         * @version 1.0
         */
        virtual ~ConfigStatistics();
    };
}

#endif // EQ_CONFIGSTATISTICS_H
