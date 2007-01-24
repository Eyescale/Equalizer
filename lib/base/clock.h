
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */


#ifndef EQBASE_CLOCK_H
#define EQBASE_CLOCK_H

#include <eq/base/base.h>

#ifdef Darwin
// http://developer.apple.com/qa/qa2004/qa1398.html
#  include <mach/mach_time.h>
#endif

#ifdef WIN32
#  include <Windows.h>
#else
#  include <time.h>
#endif

namespace eqBase
{
/** 
 * A class for time measurements.
 */
    class EQ_EXPORT Clock
    {
    public :
        /** 
         * Constructs a new clock.
         */
        Clock() 
            {
                reset();
#ifdef Darwin
                mach_timebase_info( &_timebaseInfo );
#elif defined (WIN32)
                QueryPerformanceFrequency( &_frequency );
#endif
            }

        /** 
         * Destroys the clock.
         */
        virtual ~Clock() {}

        /** 
         * Resets the base time of the clock to the current time.
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

        /** 
         * Set an alarm.
         * 
         * @param time The time in milliseconds when the alarm happens.
         */
        void setAlarm( const float time )
            {
                reset();
#ifdef Darwin
                _start += time / _timebaseInfo.numer * _timebaseInfo.denom *
                    1000000.f;
#elif defined (WIN32)
                _start.QuadPart += static_cast<long long>(time * 0.001f);
#else
                const int sec   = static_cast<int>( time * 0.001f );
                _start.tv_sec  += sec;
                _start.tv_nsec += (static_cast<int>(time) - sec) * 1000000;
#endif
            }

        /** 
         * Returns the time elapsed since the last clock reset.
         * 
         * @return the elapsed time in milliseconds
         */
        float getTimef() const
            {
#ifdef Darwin
                const uint64_t elapsed = mach_absolute_time() - _start;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000.f );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                return 1000.0f * (now.QuadPart - _start.QuadPart) / _frequency.QuadPart;
#else
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return ( 1000.0f * (now.tv_sec - _start.tv_sec) +
                         0.000001f * (now.tv_nsec - _start.tv_nsec));
#endif
            }

        /** 
         * Returns the time elapsed since the last clock reset.
         * 
         * @return the elapsed time in milliseconds
         */
        double getTimed() const
            {
#ifdef Darwin
                const uint64_t elapsed = mach_absolute_time() - _start;
                return ( elapsed * _timebaseInfo.numer / _timebaseInfo.denom /
                         1000000. );
#elif defined (WIN32)
                LARGE_INTEGER now;
                QueryPerformanceCounter( &now );
                return 1000.0 * (now.QuadPart - _start.QuadPart) / _frequency.QuadPart;
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
         */
        float getMSf() const
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
#endif  // EQBASE_CLOCK_H
