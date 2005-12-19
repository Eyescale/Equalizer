
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */


#ifndef EQBASE_CLOCK_H
#define	EQBASE_CLOCK_H

#ifdef sgi
# include <sys/syssgi.h>
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
                gettimeofday( &_start , 0 );
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
                struct timeval now;
                gettimeofday( &now , 0 );
                return (((float)now.tv_sec - (float)_start.tv_sec)  * 1000.f +
                        ((float)now.tv_usec - (float)_start.tv_usec) / 1000.f);
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
                struct timeval now;
                gettimeofday( &now , 0 );
                return 
                    (((double)now.tv_sec - (double)_start.tv_sec)  * 1000. +
                     ((double)now.tv_usec - (double)_start.tv_usec) / 1000.);
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
                struct timeval now;
                gettimeofday( &now , 0 );

                if( now.tv_usec < _start.tv_usec )
                    return ( 1000. + ((float)now.tv_usec -
                                      (float)_start.tv_usec) / 1000.f );

                return (((float)now.tv_usec - (float)_start.tv_usec) / 1000.f );
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
        struct timeval  _start;
#  else // Darwin
        struct timespec _start;
#endif // Darwin
    };
}
#endif	// EQBASE_CLOCK_H
