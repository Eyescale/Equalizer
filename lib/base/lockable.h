
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_LOCKABLE_H
#define EQBASE_LOCKABLE_H

#include <eq/base/lock.h>
#include <eq/base/nonCopyable.h>

namespace eq
{
namespace base
{
    /**
     * A convenience structure to hold data together with a lock for access.
     * 
     * Locking the data still has to be done manually, e.g, using a ScopedMutex.
     */
    template< typename D > class Lockable : public NonCopyable
    {
    public:
        /** Construct a new data structure. */
        explicit Lockable() {}

        /** Construct and initialize a new data structure. */
        explicit Lockable( const D& value ) : data( value ) {}

        /** Access the held data. */
        D* operator->() { return &data; }
        /** Access the held data. */
        const D* operator->() const { return &data; }

        D    data;
        mutable Lock lock;
    };
}
}
#endif // EQBASE_LOCKABLE_H
