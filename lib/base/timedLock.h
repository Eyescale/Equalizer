
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

#ifndef EQBASE_TIMEDLOCK_H
#define EQBASE_TIMEDLOCK_H

#include <eq/base/base.h>
#include <eq/base/nonCopyable.h>

namespace eq
{
namespace base
{
    class TimedLockPrivate;

    /**
     * A mutex with timeout capabilities.
     */
    class TimedLock : public NonCopyable
    {
    public:
        /** Constructs a new timed lock. */
        EQ_EXPORT TimedLock();

        /** Destructs the lock. */
        EQ_EXPORT ~TimedLock();

        /** 
         * Set the lock. 
         * 
         * @param timeout the timeout in milliseconds to wait for the lock,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return <code>true</code> if the lock was acquired,
         *         <code>false</code> if not.
         */
        EQ_EXPORT bool set( const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /** 
         * Releases the lock.
         */
        EQ_EXPORT void unset();

        /** 
         * Attempts to set the lock.
         * 
         * @return <code>true</code> if the lock was acquired,
         *         <code>false</code> if not.
         */
        EQ_EXPORT bool trySet();

        /** 
         * Tests if the lock is set.
         * 
         * @return <code>true</code> if the lock is set,
         *         <code>false</code> if it is not set.
         */
        EQ_EXPORT bool test(); 

    private:
        TimedLockPrivate* _data;
        bool              _locked;
    };
}
}
#endif //EQBASE_TIMEDLOCK_H
