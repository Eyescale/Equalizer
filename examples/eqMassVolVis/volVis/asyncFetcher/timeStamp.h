
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__TIME_STAMP_H
#define MASS_VOL__TIME_STAMP_H

#include <inttypes.h> // uint64_t
#include <msv/types/limits.h>

namespace massVolVis
{

struct TimeStamp
{
    explicit TimeStamp( uint64_t time_ = 0 ) : _time( time_ ){}

    void reset() { _time = 0; }

    void increment() { ++_time; }
    void increment( uint64_t delta ) { _time += delta; }

    bool operator== ( const TimeStamp& other ) const { return _time == other._time; }
    bool operator!= ( const TimeStamp& other ) const { return _time != other._time; }
    bool operator<  ( const TimeStamp& other ) const { return _time <  other._time; }

    static TimeStamp max()
        { return TimeStamp( Limits< uint64_t >::max() ); }

    bool isNull() const { return _time == 0; }

private:
    uint64_t _time;
};


}//namespace massVolVis

#endif //MASS_VOL__TIME_STAMP_H

