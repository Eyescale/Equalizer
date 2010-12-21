
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_VERSION_H
#define CO_VERSION_H

#include <co/types.h>
#include <co/api.h>
namespace co
{
/** Special object version values */
extern CO_API const uint128_t VERSION_NONE;
extern CO_API const uint128_t VERSION_FIRST;
extern CO_API const uint128_t VERSION_NEXT;
extern CO_API const uint128_t VERSION_INVALID;
extern CO_API const uint128_t VERSION_OLDEST;
extern CO_API const uint128_t VERSION_HEAD;

}

#endif // CO_VERSION_H
