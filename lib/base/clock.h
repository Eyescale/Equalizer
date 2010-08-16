
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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


#ifndef EQBASE_CLOCK_H
#define EQBASE_CLOCK_H

#include <eq/base/base.h>

#ifdef Darwin
// http://developer.apple.com/qa/qa2004/qa1398.html
#  include <mach/mach_time.h>
#endif

#ifdef WIN32
#  include <windows.h>
#else
#  include <time.h>
#endif

namespace eq
{
namespace base
{
    /** A class for time measurements. */
    class Clock
    {
    public :
        /** Construct a new clock. @version 1.0 */
        Clock() 
            {
                reset();
#ifdef Darwin
                mach_timebase_info( &_timebaseInfo );
#elif defined (WIN32)
                QueryPerformanceFrequency( &_frequency );
#endif
            }

        /** Destroy the clock. @version 1.0 */
        ~Clock() {}

        /**
         * Reset the base time of the clock to the current time.
         * @version 1.0
         */
        void reset()
            {
#ifdef Darwin
                _start = mach_absolute_time();
#elif defined (WIN32)
                QueryPerformanceCounter( &_start );
#else
                clock_gettime( CLOCK_REALTIME, &_start );
#endif
            }

        /** Set the current time of the clock. @version 1.0 */
        void set( const int64_t time )
            {
                reset();
#ifdef Darwin
                _start -= static_cast< uint64_t >(
                              time * _timebaseInfo.denom / _timebaseInfo.numer *
                                     1000000 );
#elif defined (WIN32)
                _start.QuadPart -= static_cast<long long>( 
                    time * _frequency.QuadPart / 1000 );
#else
                const int sec   = static_cast< int >( time / 1000 ) + 1;
                _start.tv_sec  -= sec;
                _start.tv_nsec -= static_cast<int>(
                    (time - sec * 1000) * 1000000 );
                if( _start.tv_nsec > 1000000000 )
                {
                    _start.tv_sec  += 1;
                    _start.tv_nsec -= 1000000000;
                }
#endif
            }

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        float getTimef() const
            {
#ifdef Darwin
                const int64_t elapsed = mach_absolute_time() - _start;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000.f );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                return 1000.0f * (now.QuadPart - _start.QuadPart) / 
                    _frequency.QuadPart;
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return ( 1000.0f * (now.tv_sec - _start.tv_sec) +
                         0.000001f * (now.tv_nsec - _start.tv_nsec));
#endif
            }

        /** 
         * @return the elapsed time in milliseconds since the last clock reset
         *         and atomically reset the clock.
         * @version 1.0
         */
        float resetTimef()
            {
#ifdef Darwin
                const uint64_t now = mach_absolute_time();
                const int64_t elapsed = now - _start;
                _start = now;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000.f );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                const float time = 1000.0f * (now.QuadPart - _start.QuadPart) / 
                                   _frequency.QuadPart;
                _start = now;
                return time;
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                const float time = ( 1000.0f * (now.tv_sec - _start.tv_sec) +
                                    0.000001f * (now.tv_nsec - _start.tv_nsec));
                _start = now;
                return time;
#endif
            }

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        int64_t getTime64() const
            {
#ifdef Darwin
                const int64_t elapsed = mach_absolute_time() - _start;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000 );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                return 1000 * (now.QuadPart - _start.QuadPart) /
                    _frequency.QuadPart;
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return ( 1000 * (now.tv_sec - _start.tv_sec) +
                         static_cast< int64_t >(0.000001f * 
                                                 (now.tv_nsec-_start.tv_nsec)));
#endif
            }

        /** 
         * @return the elapsed time in milliseconds since the last clock reset.
         * @version 1.0
         */
        double getTimed() const
            {
#ifdef Darwin
                const int64_t elapsed = mach_absolute_time() - _start;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000. );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                return 1000.0 * (now.QuadPart - _start.QuadPart) /
                                                            _frequency.QuadPart;
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return ( 1000.0 * (now.tv_sec - _start.tv_sec) +
                         0.000001 * (now.tv_nsec - _start.tv_nsec));
#endif
            }

        /** 
         * Returns the millisecond part of the time elapsed since the last
         * reset. 
         * 
         * Obviously the returned time overflows once per second.
         * 
         * @return the millisecond part of the time elapsed. 
         * @version 1.0
         */
        float getMilliSecondsf() const
            {
#if defined (Darwin) || defined (WIN32)
                double time = getTimed();
                return static_cast<float>
                    (time - static_cast<unsigned>(time/1000.) * 1000);
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );

                if( now.tv_nsec < _start.tv_nsec )
                    return ( 1000.f + 0.000001f*(now.tv_nsec - _start.tv_nsec));
                
                return ( 0.000001f * ( now.tv_nsec - _start.tv_nsec ));
#endif
            }

    private:
#ifdef Darwin
        uint64_t                  _start;
        mach_timebase_info_data_t _timebaseInfo;
#elif defined (WIN32)
        LARGE_INTEGER             _start;
        LARGE_INTEGER             _frequency;
#else
        struct timespec _start;
#endif
    };
}
}
#endif  // EQBASE_CLOCK_H
