
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/base/lock.h>
#include <eq/base/nonCopyable.h>

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
    class ScopedMutex : public NonCopyable
    {
    public:
        /** 
         * Constructs a new scoped mutex using the given lock.
         * 
         * @param lock the mutex to set and unset, or 0.
         */
        explicit ScopedMutex( Lock* lock ) : _lock( lock )
            { if( lock ) lock->set(); }

        /** Constructs a new scoped mutex using the given lock. */
        explicit ScopedMutex( Lock& lock ) : _lock( &lock )
            { lock.set(); }

        /** Destructs the scoped mutex and unsets the mutex. */
        ~ScopedMutex() { if( _lock ) _lock->unset(); }

    private:
        ScopedMutex(){}
        Lock* _lock;
    };
}
}
#endif //EQBASE_SCOPEDMUTEX_H
