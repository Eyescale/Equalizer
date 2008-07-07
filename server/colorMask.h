
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_COLORMASK_H
#define EQSERVER_COLORMASK_H

#include "compound.h"

#include <eq/client/colorMask.h>
#include <iostream>

namespace eq
{
namespace server
{
    /**
     * Extends eq::Colormask with functionality needed only by the server.
     */
    class ColorMask : public eq::ColorMask
    {
    public:
        ColorMask(){}
        ColorMask( const int attribute ) 
        {
            red   = (attribute & Compound::COLOR_MASK_RED);
            green = (attribute & Compound::COLOR_MASK_GREEN);
            blue  = (attribute & Compound::COLOR_MASK_BLUE);
        }     
    };
}
}
#endif // EQ_COLORMASK_H
