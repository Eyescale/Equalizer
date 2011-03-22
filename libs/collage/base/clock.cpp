
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
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


#include "clock.h"

#include "os.h"

#include <co/base/os.h>
#ifdef Darwin
// http://developer.apple.com/qa/qa2004/qa1398.html
#  include <mach/mach_time.h>
#endif
#ifndef _WIN32
#  include <time.h>
#endif

namespace co
{
namespace base
{
class ClockPrivate
{
public:
#ifdef Darwin
    uint64_t start;
    mach_timebase_info_data_t timebaseInfo;
#elif defined (_WIN32)
    LARGE_INTEGER start;
    LARGE_INTEGER frequency;
#else
    struct timespec start;
#endif
};

Clock::Clock()
        : _data( new ClockPrivate )
{
    reset();
#ifdef Darwin
    mach_timebase_info( &_data->timebaseInfo );
#elif defined (_WIN32)
    QueryPerformanceFrequency( &_data->frequency );
#endif
}

Clock::~Clock()
{
    delete _data;
    _data = 0;
}

void Clock::reset()
{
#ifdef Darwin
    _data->start = mach_absolute_time();
#elif defined (_WIN32)
    QueryPerformanceCounter( &_data->start );
#else
    clock_gettime( CLOCK_REALTIME, &_data->start );
#endif
}

void Clock::set( const int64_t time )
{
    reset();
#ifdef Darwin
    _data->start -= static_cast< uint64_t >(
        time * _data->timebaseInfo.denom / _data->timebaseInfo.numer *
                                     1000000 );
#elif defined (_WIN32)
    _data->start.QuadPart -= static_cast<long long>( 
        time * _data->frequency.QuadPart / 1000 );
#else
    const int sec   = static_cast< int >( time / 1000 ) + 1;
    _data->start.tv_sec  -= sec;
    _data->start.tv_nsec -= static_cast<int>(
        (time - sec * 1000) * 1000000 );
    if( _data->start.tv_nsec > 1000000000 )
    {
        _data->start.tv_sec  += 1;
        _data->start.tv_nsec -= 1000000000;
    }
#endif
}

float Clock::getTimef() const
{
#ifdef Darwin
    const int64_t elapsed = mach_absolute_time() - _data->start;
    return ( elapsed * _data->timebaseInfo.numer / _data->timebaseInfo.denom /
             1000000.f );
#elif defined (_WIN32)
    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );
    return 1000.0f * (now.QuadPart - _data->start.QuadPart) / 
        _data->frequency.QuadPart;
#else
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
    return ( 1000.0f * (now.tv_sec - _data->start.tv_sec) +
             0.000001f * (now.tv_nsec - _data->start.tv_nsec));
#endif
}

float Clock::resetTimef()
{
#ifdef Darwin
    const uint64_t now = mach_absolute_time();
    const int64_t elapsed = now - _data->start;
    const float time = elapsed * _data->timebaseInfo.numer /
        _data->timebaseInfo.denom / 1000000.f;
#elif defined (_WIN32)
    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );
    const float time = 1000.0f * (now.QuadPart - _data->start.QuadPart) / 
        _data->frequency.QuadPart;
#else
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
    const float time = ( 1000.0f * (now.tv_sec - _data->start.tv_sec) +
                         0.000001f * (now.tv_nsec - _data->start.tv_nsec));
#endif
    _data->start = now;
    return time;
}

int64_t Clock::getTime64() const
{
#ifdef Darwin
    const int64_t elapsed = mach_absolute_time() - _data->start;
    return ( elapsed * _data->timebaseInfo.numer /
            _data->timebaseInfo.denom + 500000 ) / 1000000;
#elif defined (_WIN32)
    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );
    return ( 1000 * (now.QuadPart-_data->start.QuadPart) +
             (_data->frequency.QuadPart>>1) ) / _data->frequency.QuadPart;
#else
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
    return ( 1000 * (now.tv_sec - _data->start.tv_sec) +
             int64_t( 0.000001f * (now.tv_nsec - _data->start.tv_nsec+500000)));
#endif
}

double Clock::getTimed() const
{
#ifdef Darwin
    const int64_t elapsed = mach_absolute_time() - _data->start;
    return ( elapsed * _data->timebaseInfo.numer / _data->timebaseInfo.denom /
             1000000. );
#elif defined (_WIN32)
    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );
    return 1000.0 * (now.QuadPart - _data->start.QuadPart) /
        _data->frequency.QuadPart;
#else
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );
    return ( 1000.0 * (now.tv_sec - _data->start.tv_sec) +
             0.000001 * (now.tv_nsec - _data->start.tv_nsec));
#endif
}

float Clock::getMilliSecondsf() const
{
#if defined (Darwin) || defined (_WIN32)
    double time = getTimed();
    return static_cast<float>
        (time - static_cast<unsigned>(time/1000.) * 1000);
#else
    struct timespec now;
    clock_gettime( CLOCK_REALTIME, &now );

    if( now.tv_nsec < _data->start.tv_nsec )
        return ( 1000.f + 0.000001f*(now.tv_nsec - _data->start.tv_nsec));
                
    return ( 0.000001f * ( now.tv_nsec - _data->start.tv_nsec ));
#endif
}

}
}
