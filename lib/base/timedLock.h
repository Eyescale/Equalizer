
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

    /** A mutex with timeout capabilities. */
    class TimedLock : public NonCopyable
    {
    public:
        /** Construct a new timed lock. @version 1.0 */
        EQ_EXPORT TimedLock();

        /** Destruct the lock. @version 1.0 */
        EQ_EXPORT ~TimedLock();

        /** 
         * Set the lock. 
         * 
         * @param timeout the timeout in milliseconds to wait for the lock,
         *                or <code>EQ_TIMEOUT_INDEFINITE</code> to wait
         *                indefinitely.
         * @return true if the lock was acquired, false if not.
         * @version 1.0
         */
        EQ_EXPORT bool set( const uint32_t timeout = EQ_TIMEOUT_INDEFINITE );

        /** Release the lock. @version 1.0 */
        EQ_EXPORT void unset();

        /** 
         * Attempt to set the lock.
         * 
         * @return true if the lock was acquired, false if not.
         * @version 1.0
         */
        EQ_EXPORT bool trySet();

        /** 
         * Test if the lock is set.
         * 
         * @return true if the lock is set, false if it is not set.
         * @version 1.0
         */
        EQ_EXPORT bool isSet(); 

    private:
        TimedLockPrivate* _data;
        bool              _locked;
    };
}
}
#endif //EQBASE_TIMEDLOCK_H
