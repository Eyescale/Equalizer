
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "vec3.ipp"

namespace massVolVis
{

template struct Vec3< uint16_t >;
template struct Vec3< uint32_t >;
template struct Vec3<  int32_t >;
template struct Vec3<  int64_t >;
template struct Vec3<    float >;
template struct Vec3<   double >;

}

