
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/api.h>

#include <string>

namespace eq
{
    /** The list of possible window systems. @sa Pipe::getWindowSystem() */
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_AGL,      //!< AGL/Carbon
        WINDOW_SYSTEM_GLX,      //!< GLX/X11
        WINDOW_SYSTEM_WGL,      //!< WGL/Win32
        WINDOW_SYSTEM_ALL       // must be last
    };

    /** Print the window system name to the given output stream. @version 1.0 */
    EQ_API std::ostream& operator << ( std::ostream& os, const WindowSystem& );
}
#endif // EQ_WINDOWSYSTEM_H

