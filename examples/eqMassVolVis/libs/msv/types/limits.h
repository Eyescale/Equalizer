
/* Copyright (c) 2012, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__LIMITS
#define MASS_VOL__LIMITS

#include <limits>


namespace massVolVis
{

// fix negative limits for float types:
template< typename T >
struct Limits
{
    inline static T max() { return std::numeric_limits<T>::max(); }
    inline static T min() { return std::numeric_limits<T>::min(); }
};


template<> inline float  Limits<float >::min(){ return -std::numeric_limits<float >::max(); }
template<> inline double Limits<double>::min(){ return -std::numeric_limits<double>::max(); }


} // namespace massVolVis

#endif // MASS_VOL__LIMITS

