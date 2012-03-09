
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_SPINLOCK_H
#define COBASE_SPINLOCK_H

#include <co/base/atomic.h>         // member
#include <co/base/nonCopyable.h>    // base class
#include <co/base/thread.h>         // used in inline method

namespace co
{
namespace base
{
namespace detail { class SpinLock; }

    /** 
     * A fast lock for uncontended memory access.
     *
     * If Thread::yield does not work like expected, priority inversion is
     * possible. If used as a read-write lock, readers or writers will starve on
     * high contention.
     *
     * @sa ScopedMutex
     */
    class SpinLock : public NonCopyable
    {
    public:
        /** Construct a new lock. @version 1.0 */
        COBASE_API SpinLock();

        /** Destruct the lock. @version 1.0 */
        COBASE_API ~SpinLock();

        /** Acquire the lock exclusively. @version 1.0 */
        COBASE_API void set();

        /** Release an exclusive lock. @version 1.0 */
        COBASE_API void unset();

        /** 
         * Attempt to acquire the lock exclusively.
         *
         * @return true if the lock was set, false if
         *         it was not set.
         * @version 1.0
         */
        COBASE_API bool trySet();

        /** Acquire the lock shared with other readers. @version 1.1.2 */
        COBASE_API void setRead();

        /** Release a shared read lock. @version 1.1.2 */
        COBASE_API void unsetRead();

        /** 
         * Attempt to acquire the lock shared with other readers.
         *
         * @return true if the lock was set, false if
         *         it was not set.
         * @version 1.1.2
         */
        COBASE_API bool trySetRead();

        /**
         * Test if the lock is set.
         * 
         * @return true if the lock is set, false if
         *         it is not set.
         * @version 1.0
         */
        COBASE_API bool isSet();

        /**
         * Test if the lock is set exclusively.
         * 
         * @return true if the lock is set, false if it is not set.
         * @version 1.1.2
         */
        COBASE_API bool isSetWrite();

        /**
         * Test if the lock is set shared.
         * 
         * @return true if the lock is set, false if it is not set.
         * @version 1.1.2
         */
        COBASE_API bool isSetRead();

    private:
        detail::SpinLock* const _impl;
    };
}

}
#endif //COBASE_SPINLOCK_H
