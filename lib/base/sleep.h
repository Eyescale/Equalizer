
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_SLEEP_H
#define EQBASE_SLEEP_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    inline void sleep( const uint32_t milliSeconds )
    {
#ifdef WIN32_VC
        ::Sleep( milliSeconds );
#else
        ::usleep( milliSeconds * 1000 );
#endif
    }
}
}
#endif  // EQBASE_SLEEP_H
