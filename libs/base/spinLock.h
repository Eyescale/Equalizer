
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

#ifndef EQBASE_SPINLOCK_H
#define EQBASE_SPINLOCK_H

#include <eq/base/atomic.h>         // member
#include <eq/base/compareAndSwap.h> // used in inline method
#include <eq/base/nonCopyable.h>    // base class
#include <eq/base/thread.h>         // used in inline method

namespace eq
{
namespace base
{
    /** 
     * A fast lock for uncontended memory access.
     *
     * Be aware of possible priority inversion.
     *
     * @sa ScopedMutex
     */
    class SpinLock : public NonCopyable
    {
    public:
        /** Construct a new lock. @version 1.0 */
        SpinLock() : _set( 0 ) {}

        /** Destruct the lock. @version 1.0 */
        ~SpinLock() { _set = 0; }

        /** Acquire the lock. @version 1.0 */
        void set()
            {
                while( true )
                {
                    for( unsigned i=0; i < 100; ++i )
                    {
                        if( compareAndSwap( &_set, 0, 1 ))
                            return;
                        Thread::yield();
                    }
                }
            }

        /** Release the lock. @version 1.0 */
        void unset() { _set = 0; memoryBarrier(); }

        /** 
         * Attempt to acquire the lock.
         *
         * This method implements an atomic test-and-set operation.
         *
         * @return <code>true</code> if the lock was set, <code>false</code> if
         *         it was not set.
         * @version 1.0
         */
        bool trySet()
            {
                if( compareAndSwap( &_set, 0, 1 ))
                    return true;
                return false;
            }

        /** 
         * Test if the lock is set.
         * 
         * @return <code>true</code> if the lock is set, <code>false</code> if
         *         it is not set.
         * @version 1.0
         */
        bool isSet() { memoryBarrier(); return ( _set == 1 ); }

    private:
        long _set;
    };
}

}
#endif //EQBASE_SPINLOCK_H
