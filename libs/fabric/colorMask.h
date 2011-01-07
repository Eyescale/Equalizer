
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_COLORMASK_H
#define EQFABRIC_COLORMASK_H

#include <eq/fabric/api.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /**
     * Defines which parts of the color buffer are to be written.
     *
     * Used to configure anaglyphic stereo rendering.
     */
    class ColorMask
    {
    public:
        /** Construct a color mask with all components enabled. @version 1.0 */
        ColorMask() : red( true ), green( true ), blue( true ), alpha( true ) {}
        
        /** Construct a color mask with given default values. @version 1.0 */
        ColorMask( const bool r, const bool g, const bool b, 
                   const bool a = true )
                : red( r ), green( g ), blue( b ), alpha( a ) {}

        bool red;
        bool green;
        bool blue;
        bool alpha;

        EQFABRIC_API static const ColorMask ALL;
    };

    inline std::ostream& operator << ( std::ostream& os, const ColorMask& mask )
    {
        os << "[ ";
        if( mask.red ) 
            os << "RED ";
        if( mask.green ) 
            os << "GREEN ";
        if( mask.blue ) 
            os << "BLUE ";
        os << "]";

        return os;
    }
}
}

#endif // EQFABRIC_COLORMASK_H
