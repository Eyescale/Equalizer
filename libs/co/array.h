
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef CO_ARRAY_H
#define CO_ARRAY_H

#include <co/types.h>

namespace co
{
    /** A wrapper to (de)serialize arrays. */
    template< class T > class Array
    {
    public:
        /** Create a new array wrapper for the given data. @version 1.0 */
        explicit Array( T* data_, const size_t num_ )
                : data( data_ ), num( num_ ) {}

        /** @return the number of bytes stored in the pointer. @version 1.0 */
        size_t getNumBytes() const { return num * sizeof( T ); }

        // for #146: void swapByteOrder() {...} 

        T* const data; //!< The data
        const size_t num; //!<  The number of elements in the data
    };

    template<> inline size_t Array< void >::getNumBytes() const { return num; }
}

#endif // CO_ARRAY_H

