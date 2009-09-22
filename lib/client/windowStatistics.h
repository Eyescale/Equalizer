
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WINDOWSTATISTICS_H
#define EQ_WINDOWSTATISTICS_H

#include <eq/client/statisticSampler.h> // base class

namespace eq
{
    class Window;

    /** A statistics sampler for window-related events.
     */
    class WindowStatistics : public StatisticSampler< Window >
    {
    public:
        WindowStatistics( const Statistic::Type type, Window* window );
        virtual ~WindowStatistics();
    };
}

#endif // EQ_WINDOWSTATISTICS_H
