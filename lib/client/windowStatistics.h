
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/event.h> // member

namespace eq
{
    class Window;

    /**
     * Holds one statistics event, used for profiling.
     */
    class WindowStatistics
    {
    public:
        WindowStatistics( const Statistic::Type type, Window* window );
        ~WindowStatistics();

    private:
        Event         _event;
        Window* const _window;
    };
}

#endif // EQ_WINDOWSTATISTICS_H
