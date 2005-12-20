
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */


#ifndef EQBASE_CLOCK_H
#define	EQBASE_CLOCK_H

#ifdef Darwin
// http://developer.apple.com/qa/qa2004/qa1398.html
#  include <mach/mach_time.h>
#endif

#include <sys/time.h>

namespace eqBase
{
/** 
 * A class for time measurements.
 */
    class Clock
    {
    public :
        /** 
         * Constructs a new clock.
         */
        Clock() { reset(); }

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
                mach_timebase_info( &_timebaseInfo );
                _start = mach_absolute_time();
#else // Darwin
                clock_gettime( CLOCK_REALTIME, &_start );
#endif // Darwin
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
#else // Darwin
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return (((float)now.tv_sec - (float)_start.tv_sec) * 1000.f +
                     ((float)now.tv_nsec - (float)_start.tv_nsec) / 1000000.f);
#endif // Darwin
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
#else // Darwin
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );
                return
                    (((double)now.tv_sec - (double)_start.tv_sec) * 1000. +
                     ((double)now.tv_nsec - (double)_start.tv_nsec) /1000000. );
#endif // Darwin
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
#ifdef Darwin
                double time = getTimed();
                return (time - (uint64_t)(time/1000.) * 1000);
#else // Darwin
                struct timespec now;
                clock_gettime( CLOCK_REALTIME, &now );

                if( now.tv_nsec < _start.tv_nsec )
                    return ( 1000. + ((float)now.tv_nsec -
                                      (float)_start.tv_nsec) / 1000000.f );
                
                return (((float)now.tv_nsec - (float)_start.tv_nsec)/1000000.f);
#endif // Darwin
            }

    private:
#  ifdef Darwin
        uint64_t                  _start;
        mach_timebase_info_data_t _timebaseInfo;
#  else // Darwin
        struct timespec _start;
#endif // Darwin
    };
}
#endif	// EQBASE_CLOCK_H
