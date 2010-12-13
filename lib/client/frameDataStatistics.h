
/* Copyright (c)      2008, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#ifndef EQ_FRAMEDATASTATISTICS_H
#define EQ_FRAMEDATASTATISTICS_H

#include <eq/client/statisticSampler.h> // base class

namespace eq
{
    class FrameData;

    /**
     * Holds one statistics event, used for profiling.
     */
    class FrameDataStatistics : public StatisticSampler< FrameData >
    {
    public:
        FrameDataStatistics( const Statistic::Type type, FrameData* frameData,
                             Node* node, const uint32_t frameNumber,
                             const uint128_t& originator );
        ~FrameDataStatistics();
    private:
        Node* _node;
    };
}

#endif // EQ_FRAMEDATASTATISTICS_H
