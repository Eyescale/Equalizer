
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_CHANNELSTATISTICS_H
#define EQ_CHANNELSTATISTICS_H

#include <eq/statisticSampler.h> // base class
#include <eq/types.h>
#include <eq/fabric/iAttribute.h>

namespace eq
{
/** Samples one channel statistics event. */
class ChannelStatistics : public StatisticSampler< Channel >
{
public:
    /**
     * Construct a statistics sampler and sample the start time.
     * @version 1.0
     */
    explicit EQ_API ChannelStatistics( const Statistic::Type type,
                                       Channel* channel,
                                       uint32_t frame = LB_UNDEFINED_UINT32,
                                       const int32_t hint = AUTO );
    /**
     * Destruct the sampler, sample the end time and send the event.
     * @version 1.0
     */
    virtual EQ_API ~ChannelStatistics();

private:
    int32_t _hint;
};
}

#endif // EQ_CHANNELSTATISTICS_H
