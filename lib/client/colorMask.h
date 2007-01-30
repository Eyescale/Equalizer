
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_COLORMASK_H
#define EQ_COLORMASK_H

#include <eq/base/base.h>
#include <iostream>

namespace eq
{
    /**
     * Defines which parts of the color buffer are to be written.
     */
    class EQ_EXPORT ColorMask
    {
    public:
        ColorMask() : red( true ), green( true ), blue( true ) {}
        ColorMask( const bool r, const bool g, const bool b )
            : red( r ), green( g ), blue( b ) {}

        bool red;
        bool green;
        bool blue;

        static const ColorMask ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const ColorMask& mask )
    {
        os << "[ ";
        if( mask.red ) 
            os << "red ";
        if( mask.green ) 
            os << "green ";
        if( mask.blue ) 
            os << "blue ";
        os << "]";

        return os;
    }
}

#endif // EQ_COLORMASK_H
