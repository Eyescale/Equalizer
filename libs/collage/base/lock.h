
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_LOCK_H
#define COBASE_LOCK_H

#include <co/base/api.h>
#include <co/base/nonCopyable.h>

namespace co
{
namespace base
{
    class LockPrivate;

    /** 
     * A lock (mutex) primitive.
     * @sa ScopedMutex
     */
    class Lock : public NonCopyable
    {
    public:
        /** Construct a new lock. @version 1.0 */
        COBASE_API Lock();

        /** Destruct the lock. @version 1.0 */
        COBASE_API ~Lock();

        /** Acquire the lock. @version 1.0 */
        COBASE_API void set();

        /** Release the lock. @version 1.0 */
        COBASE_API void unset();

        /** 
         * Attempt to acquire the lock.
         *
         * This method implements an atomic test-and-set operation.
         *
         * @return <code>true</code> if the lock was set, <code>false</code> if
         *         it was not set.
         * @version 1.0
         */
        COBASE_API bool trySet();

        /** 
         * Test if the lock is set.
         * 
         * @return <code>true</code> if the lock is set, <code>false</code> if
         *         it is not set.
         * @version 1.0
         */
        COBASE_API bool isSet();

    private:
        LockPrivate* _data;
    };
}

}
#endif //COBASE_LOCK_H
