
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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


#ifndef COBASE_CLOCK_H
#define COBASE_CLOCK_H

#include <co/base/api.h>
#include <co/base/types.h>

namespace co
{
namespace base
{
    class ClockPrivate;

    /** A class for time measurements. */
    class Clock
    {
    public :
        /** Construct a new clock. @version 1.0 */
        COBASE_API Clock();

        /** Destroy the clock. @version 1.0 */
        COBASE_API ~Clock();

        /**
         * Reset the base time of the clock to the current time.
         * @version 1.0
         */
        COBASE_API void reset();

        /** Set the current time of the clock. @version 1.0 */
        COBASE_API void set( const int64_t time );

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        COBASE_API float getTimef() const;

        /** 
         * @return the elapsed time in milliseconds since the last clock reset
         *         and atomically reset the clock.
         * @version 1.0
         */
        COBASE_API float resetTimef();

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        COBASE_API int64_t getTime64() const;

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        COBASE_API double getTimed() const;

        /** 
         * Returns the millisecond part of the time elapsed since the last
         * reset. 
         * 
         * Obviously the returned time overflows once per second.
         * 
         * @return the millisecond part of the time elapsed. 
         * @version 1.0
         */
        COBASE_API float getMilliSecondsf() const;

    private:
        ClockPrivate* _data;
    };
}
}
#endif  // COBASE_CLOCK_H
