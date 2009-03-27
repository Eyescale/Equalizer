
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

#ifndef EQBASE_LOCK_H
#define EQBASE_LOCK_H

#include <eq/base/base.h>

namespace eq
{
namespace base
{
    class LockPrivate;

    /**
     * A lock (mutex) primitive.
     */
    class EQ_EXPORT Lock 
    {
    public:
        /** 
         * Constructs a new lock.
         */
        Lock();


        /** Destructs the lock. */
        ~Lock();

        /** 
         * Sets the lock. 
         */
        void set();

        /** 
         * Releases the lock.
         */
        void unset();

        /** 
         * Attempts to set the lock.
         * 
         * @return <code>true</code> if the lock was set, <code>false</code> if
         *         it was not set.
         */
        bool trySet();

        /** 
         * Tests if the lock is set.
         * 
         * @return <code>true</code> if the lock is set, <code>false</code> if
         *         it is not set.
         */
        bool test(); 

    private:
        LockPrivate* _data;
    };
}

}
#endif //EQBASE_LOCK_H
