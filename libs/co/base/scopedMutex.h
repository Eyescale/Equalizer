
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef COBASE_SCOPEDMUTEX_H
#define COBASE_SCOPEDMUTEX_H

#include <co/base/lock.h>        // used in inline method
#include <co/base/lockable.h>    // used in inline method
#include <co/base/nonCopyable.h> // base class
#include <co/base/types.h>

namespace co
{
namespace base
{
    class WriteOp;
    class ReadOp;

    /** @cond IGNORE */
    template< class L, class T > struct ScopedMutexLocker {};
    template< class L > struct ScopedMutexLocker< L, WriteOp >
    {
        static inline void set( L& lock ) { lock.set(); }
        static inline void unset( L& lock ) { lock.unset(); }
    };
    template< class L > struct ScopedMutexLocker< L, ReadOp >
    {
        static inline void set( L& lock ) { lock.setRead(); }
        static inline void unset( L& lock ) { lock.unsetRead(); }
    };
    /** @endcond */

    /**
     * A scoped mutex.
     * 
     * The mutex is automatically set upon creation, and released when the
     * scoped mutex is destroyed, e.g., when the scope is left. The scoped mutex
     * does nothing if a 0 pointer for the lock is passed.
     */
    template< class L = Lock, class T = WriteOp >
    class ScopedMutex : public NonCopyable
    {
        typedef ScopedMutexLocker< L, T > LockTraits;

    public:
        /** 
         * Construct a new scoped mutex and set the given lock.
         *
         * Providing no Lock (0) is allowed, in which case the scoped mutex does
         * nothing.
         *
         * @param lock the mutex to set and unset, or 0.
         * @version 1.0
         */
        explicit ScopedMutex( L* lock ) : _lock( lock )
            { if( lock ) LockTraits::set( *lock ); }

        /** Construct a new scoped mutex and set the given lock. @version 1.0 */
        explicit ScopedMutex( L& lock ) : _lock( &lock )
            { LockTraits::set( lock ); }

        /**
         * Construct a new scoped mutex for the given Lockable data structure.
         * @version 1.0
         */
        template< typename LB > ScopedMutex( LB& lockable )
                : _lock( &lockable.lock ) { LockTraits::set( lockable.lock ); }

        /** Destruct the scoped mutex and unset the mutex. @version 1.0 */
        ~ScopedMutex() { leave(); }

        /** Leave and unlock the mutex immediately. @version 1.0 */
        void leave() { if( _lock ) LockTraits::unset( *_lock ); _lock = 0; }

    private:
        ScopedMutex();
        L* _lock;
    };

    /** A scoped mutex for a fast uncontended read operation. @version 1.1.2 */
    typedef ScopedMutex< SpinLock, ReadOp > ScopedFastRead;

    /** A scoped mutex for a fast uncontended write operation. @version 1.1.2 */
    typedef ScopedMutex< SpinLock, WriteOp > ScopedFastWrite;

    /** A scoped mutex for a read operation. @version 1.1.5 */
    typedef ScopedMutex< Lock, ReadOp > ScopedRead;

    /** A scoped mutex for a write operation. @version 1.1.5 */
    typedef ScopedMutex< Lock, WriteOp > ScopedWrite;
}
}
#endif //COBASE_SCOPEDMUTEX_H
