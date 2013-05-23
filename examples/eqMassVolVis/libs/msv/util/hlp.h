
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef UTIL_HLP_H
#define UTIL_HLP_H

#include <msv/types/types.h>
#include <string>
#include <vector>

namespace hlpFuncs
{

namespace
{
uint32_t getPow2( int32_t val )
{
    if( val <= 0 )
        return 1;
    if( val >= (1 << 30) )
        throw "too large value";

    val--;
    val = (val >> 1)  | val;
    val = (val >> 2)  | val;
    val = (val >> 4)  | val;
    val = (val >> 8)  | val;
    val = (val >> 16) | val;
    val++;

    return val;
}
}

template < typename T >
inline T cubed( const T& val )
{
    return val*val*val;
}


template < typename T >
inline T myMin( const T& a, const T& b )
{
    if( a < b ) return a;

    return b;
}


template < typename T >
inline T myMax( const T& a, const T& b )
{
    if( a > b ) return a;

    return b;
}


template < class T >
T myClip( const T& val, const T& mn, const T& mx )
{
    if( val < mn ) return mn;
    if( val > mx ) return mx;

    return val;
}


template <typename T>
void clearVector( std::vector<T>& vec )
{
    memset( &vec[0], 0, vec.size()*sizeof( T )); 
}
}

#endif //UTIL_HLP_H
