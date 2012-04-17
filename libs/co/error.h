
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef CO_ERROR_H
#define CO_ERROR_H

#include <co/api.h>
#include <co/defines.h> // EQ_KB definitions

namespace co
{
    /** Defines errors produced by Collage base classes. */
    enum Error
    {
        ERROR_NONE = 0,
        ERROR_CUSTOM = LB_16KB,  // 0x4000
    };

    /** Print the error in a human-readable format. @version 1.0 */
    CO_API std::ostream& operator << ( std::ostream& os, const Error& );
}
#endif // CO_ERROR_H
