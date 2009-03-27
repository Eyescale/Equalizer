
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_SEMA_H
#define EQBASE_SEMA_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    class SemaPrivate;

    /**
     * A semaphore primitive.
     */
    class Sema 
    {
    public:
        /** 
         * Constructs a new semaphore.
         */
        Sema();


        /** Destructs the sema. */
        ~Sema();

        /** 
         * Post (v) operation.
         */
        void post();

        /** 
         * Wait (p) operation. 
         */
        void wait();

        /** 
         * Bulk post or wait.
         * 
         * @param delta the resource delta to be applied.
         */
        void adjust( const int delta );

    private:
        SemaPrivate*   _data;
        uint32_t       _value;
    };
}
}
#endif //EQBASE_SEMA_H
