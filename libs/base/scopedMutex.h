
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_SCOPEDMUTEX_H
#define EQBASE_SCOPEDMUTEX_H

#include <eq/base/lock.h>        // used in inline method
#include <eq/base/lockable.h>    // used in inline method
#include <eq/base/nonCopyable.h> // base class


namespace eq
{
namespace base
{
    /**
     * A scoped mutex.
     * 
     * The mutex is automatically set upon creation, and released when the
     * scoped mutex is destroyed, e.g., when the scope is left. The scoped mutex
     * does nothing if a 0 pointer for the lock is passed.
     */
    template< class L = Lock > class ScopedMutex : public NonCopyable
    {
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
            { if( lock ) lock->set(); }

        /** Construct a new scoped mutex and set the given lock. @version 1.0 */
        explicit ScopedMutex( L& lock ) : _lock( &lock )
            { lock.set(); }

        /**
         * Construct a new scoped mutex for the given Lockable data structure.
         * @version 1.0
         */
        template< typename LB > ScopedMutex( LB& lockable )
                : _lock( &lockable.lock ) { _lock->set(); }

        /** Destruct the scoped mutex and unset the mutex. @version 1.0 */
        ~ScopedMutex() { leave(); }

        /** Leave and unlock the mutex immediately. @version 1.0 */
        void leave() { if( _lock ) _lock->unset(); _lock = 0; }

    private:
        ScopedMutex() : _lock( 0 ) {}
        L* _lock;
    };
}
}
#endif //EQBASE_SCOPEDMUTEX_H
