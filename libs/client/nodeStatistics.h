
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

#ifndef EQ_NODESTATISTICS_H
#define EQ_NODESTATISTICS_H

#include <eq/statisticSampler.h> // base class

namespace eq
{
    class Node;

    /**
     * Holds one statistics event, used for profiling.
     */
    class NodeStatistics : public StatisticSampler< Node >
    {
    public:
        NodeStatistics( const Statistic::Type type, Node* node,
                        const uint32_t frameNumber );
        ~NodeStatistics();
    };
}

#endif // EQ_NODESTATISTICS_H
