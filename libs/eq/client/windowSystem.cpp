
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

#include "windowSystem.h"

#include <co/base/debug.h>

namespace eq
{
std::ostream& operator << ( std::ostream& os, const WindowSystem& ws )
{
    if( ws >= WINDOW_SYSTEM_ALL )
        os << "unknown (" << static_cast<unsigned>( ws ) << ')';
    else 
        os << ( ws == WINDOW_SYSTEM_NONE ? "none" :
                ws == WINDOW_SYSTEM_AGL  ? "agl"  :
                ws == WINDOW_SYSTEM_GLX  ? "glX"  :
                ws == WINDOW_SYSTEM_WGL  ? "wgl"  : "error" );

    return os;
}
}
