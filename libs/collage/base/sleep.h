
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_SLEEP_H
#define COBASE_SLEEP_H

#include <co/base/api.h>
#include <co/base/types.h>

namespace co
{
namespace base
{
    /** Sleep the current thread for a number of milliseconds. @version 1.0 */
    COBASE_API void sleep( const uint32_t milliSeconds );
}
}
#endif  // COBASE_SLEEP_H
