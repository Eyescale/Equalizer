
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
