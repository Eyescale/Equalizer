
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__BOX_N_H
#define MASS_VOL__BOX_N_H

#include <msv/types/box.h>

namespace massVolVis
{

class BoxN_f
{
public:
    explicit BoxN_f( const Box_f& b, int32_t p ) : bb( b ), pos( p ){}
    Box_f    bb;
    uint32_t pos;
};

typedef std::vector<BoxN_f> vecBoxN_f;

} //namespace massVolVis

#endif //MASS_VOL__BOX_N_H

