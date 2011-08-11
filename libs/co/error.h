
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#include <co/base/error.h> // base enum

namespace co
{
    /** Defines errors produced by Collage classes. */
    enum Error
    {
        // ERROR_FOO = co::base::ERROR_CUSTOM, // 0x8000
        ERROR_CUSTOM = EQ_32KB, // 0x8000
    };
}
#endif // COBASE_ERROR_H
