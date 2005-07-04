
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */


#ifndef EQBASE_CLOCK_H
#define	EQBASE_CLOCK_H

#ifdef sgi
# include <sys/syssgi.h>
#endif

#include <sys/time.h>

#ifdef CLOCK_SGI_CYCLE
#  define WHICH_CLOCK CLOCK_SGI_CYCLE
#else
#  define WHICH_CLOCK CLOCK_REALTIME
#endif

/** 
 * A class for time measurements.
 */
class Clock
{
public :
    /** 
     * Constructs a new clock.
     */
    Clock()
        {
#ifdef Darwin
#else // Darwin
#ifdef CLOCK_SGI_CYCLE
            // The cycle clock on SGI machines may cycle (surprise!), which
            // happens every couple of minutes on 32 bit machines. We will
            // dedect the overflow and correct the result assuming it happened
            // only once.
            // On 64 bit machines it happens every couple of years, so we don't
            // worry about it.

            struct timespec res;
            const long      cyclesize = syssgi( SGI_CYCLECNTR_SIZE );
            clock_getres( WHICH_CLOCK, &res );
            
            if( cyclesize == 32 )
            {
                // 2^32/10^9 * res
                _resolution.tv_sec =(int)( 4.294967296l * (double)res.tv_nsec);
                _resolution.tv_nsec = (long)( 4294967296l * res.tv_nsec - 
                    1000000000l * _resolution.tv_sec );
            } 
            else
            {
                _resolution.tv_sec = 0.;
                _resolution.tv_nsec = 0.;
            }
#endif // CLOCK_SGI_CYCLE
#endif // Darwin
            reset();
        }

    /** 
     * Destroys the clock.
     */
    virtual ~Clock() {}

    /** 
     * Resets the base time of the clock to the current time.
     */
    void reset( void )   
        { 
#ifdef Darwin
            gettimeofday( &_start , 0 );
#else // Darwin
            clock_gettime( WHICH_CLOCK, &_start );
#endif // Darwin
        }

    /** 
     * Returns the time elapsed since the last clock reset.
     * 
     * @return the elapsed time in milliseconds
     */
    double getTime( void )
        {
#ifdef Darwin
            struct timeval now;
            gettimeofday( &now , 0 );
            return (((double)now.tv_sec - (double)_start.tv_sec)  * 1000. +
                    ((double)now.tv_usec - (double)_start.tv_usec) / 1000. );
#else // Darwin
            struct timespec now;
            clock_gettime( WHICH_CLOCK, &now );

#if CLOCK_SGI_CYCLE
            if( now.tv_sec < start.tv_sec ) // seconds did overflow
            {
                now.tv_sec += _resolution.tv_sec; // add the time of one cycle
                now.tv_nsec += _resolution.tv_nsec;
                
                if( now.tv_nsec > 999999999l )  // nanoseconds did overflow too
                {
                    now.tv_nsec -= 1000000000;
                    now.tv_sec  += 1;
                }
y            }
#endif // CLOCK_SGI_CYCLE 
            return (((double)now.tv_sec - (double)start.tv_sec) * 1000. +
                ((double)now.tv_nsec - (double)start.tv_nsec) / 1000000.);
#endif // Darwin
        }

private:
#  ifdef Darwin
    struct timeval _start;
#  else // Darwin
    struct timespec _resolution;
    struct timespec _start;
#endif // Darwin
};

#endif	// EQBASE_CLOCK_H
