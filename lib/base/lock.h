
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_LOCK_H
#define EQBASE_LOCK_H

#include <eq/base/base.h>
#include <eq/base/nonCopyable.h>

namespace eq
{
namespace base
{
    class LockPrivate;

    /** A lock (mutex) primitive. */
    class Lock : public NonCopyable
    {
    public:
        /** Construct a new lock. */
        EQ_EXPORT Lock();


        /** Destruct the lock. */
        EQ_EXPORT ~Lock();

        /** Acquire the lock. */
        EQ_EXPORT void set();

        /** Release the lock. */
        EQ_EXPORT void unset();

        /** 
         * Attempt to acquire the lock.
         * 
         * @return <code>true</code> if the lock was set, <code>false</code> if
         *         it was not set.
         */
        EQ_EXPORT bool trySet();

        /** 
         * Test if the lock is set.
         * 
         * @return <code>true</code> if the lock is set, <code>false</code> if
         *         it is not set.
         */
        EQ_EXPORT bool test(); 

    private:
        LockPrivate* _data;
    };
}

}
#endif //EQBASE_LOCK_H
