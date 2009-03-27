
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_BARRIER_H
#define EQBASE_BARRIER_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    class BarrierPrivate;

    /**
     * A barrier primitive.
     */
    class EQ_EXPORT Barrier 
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier();

        /** Destructs the barrier. */
        ~Barrier();

        /** 
         * Enters the barrier.
         * 
         * @param size the size of the barrier, i.e., the number of
         *             participants.
         * @return the position in which the barrier was entered.
         */
        size_t enter( const size_t size );

    private:
        BarrierPrivate *_data;
    };
}

}
#endif //EQBASE_BARRIER_H
