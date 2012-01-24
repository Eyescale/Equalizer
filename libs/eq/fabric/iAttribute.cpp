
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "iAttribute.h"

#include <iostream>

#include "eq/server/global.h"

namespace eq
{
namespace fabric
{
std::ostream& operator << ( std::ostream& os, const IAttribute value )
{
	switch( value )
    {
        case UNDEFINED:     os << "UNDEFINED"; break;
        case ON:            os << "ON"; break; 
        case OFF:           os << "OFF"; break;
        case AUTO:          os << "AUTO"; break;
        case NICEST:        os << "NICEST"; break;
        case PASSIVE:       os << "PASSIVE"; break;
        case ANAGLYPH:      os << "ANAGLYPH"; break;
        case QUAD:          os << "QUAD"; break;
        case VERTICAL:      os << "VERTICAL"; break;
        case WINDOW:        os << "window"; break;
        case PBUFFER:       os << "pbuffer"; break; 
        case ASYNC:         os << "ASYNC"; break; 
        case DRAW_SYNC:     os << "DRAW_SYNC"; break; 
        case LOCAL_SYNC:    os << "LOCAL_SYNC"; break; 
        case FBO:           os << "FBO"; break; 
        case RGBA16F:       os << "RGBA16F"; break;
        case RGBA32F:       os << "RGBA32F"; break;
        case FIXED:         os << "fixed"; break;
        case RELATIVE_TO_ORIGIN:   os << "relative_to_origin"; break;
        case RELATIVE_TO_OBSERVER: os << "relative_to_observer"; break;
        case CPU: 			os << "cpu "; break;
        default:            os << static_cast< int >( value );
    }
    return os;
}

}
}
