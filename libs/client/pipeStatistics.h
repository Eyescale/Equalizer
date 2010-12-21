
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_PIPESTATISTICS_H
#define EQ_PIPESTATISTICS_H

#include <eq/statisticSampler.h> // base class

namespace eq
{
    class Pipe;

    /**
     * Holds one statistics event, used for profiling.
     */
    class PipeStatistics : public StatisticSampler< Pipe >
    {
    public:
        PipeStatistics( const Statistic::Type type, Pipe* pipe );
        virtual ~PipeStatistics();
    };
}

#endif // EQ_PIPESTATISTICS_H
