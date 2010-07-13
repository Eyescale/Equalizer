
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQBASE_BITOPERATION_H
#define EQBASE_BITOPERATION_H

#include <eq/base/types.h>

namespace eq
{
namespace base
{
    /** @return the position of the last set bit, or -1. */
    inline int64_t getIndexOfLastBit( uint32_t value )
    {
        int64_t count = -1;
        while( value ) 
        {
          ++count;
          value >>= 1;
        }
        return count;
    }
}
}
#endif //EQBASE_BITOPERATION_H
