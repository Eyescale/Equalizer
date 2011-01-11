
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

#ifndef COBASE_BITOPERATION_H
#define COBASE_BITOPERATION_H

#include <co/base/types.h>

namespace co
{
namespace base
{
    /** @return the position of the last set bit, or -1. */
    inline int32_t getIndexOfLastBit( uint32_t value )
    {
        int32_t count = -1;
        while( value ) 
        {
          ++count;
          value >>= 1;
        }
        return count;
    }
}
}
#endif //COBASE_BITOPERATION_H
