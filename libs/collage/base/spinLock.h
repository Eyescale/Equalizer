
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
        static const long _writelocked = -1;
        static const long _unlocked = 0;

    public:
        /** Construct a new lock. @version 1.0 */
        SpinLock() : _state( _unlocked ) {}

        /** Destruct the lock. @version 1.0 */
        ~SpinLock() { _state = _unlocked; }

        /** Acquire the lock exclusively. @version 1.0 */
        void set()
            {
                while( true )
                {
                    if( trySet( ))
                        return;
                    Thread::yield();
                }
            }

        /** Release an exclusive lock. @version 1.0 */
        void unset()
            {
                EQASSERT( _state == _writelocked );
                _state = _unlocked;
            }

        /** 
         * Attempt to acquire the lock exclusively.
         *
         * @return true if the lock was set, false if
         *         it was not set.
         * @version 1.0
         */
        bool trySet()
            {
                if( !_state.compareAndSwap( _unlocked, _writelocked ))
                    return false;
                EQASSERTINFO( isSetWrite(), _state );
                return true;
            }

        /** Acquire the lock shared with other readers. @version 1.1.2 */
        void setRead()
            {
                while( true )
                {
                    if( trySetRead( ))
                        return;
                    Thread::yield();
                }
            }

        /** Release a shared read lock. @version 1.1.2 */
        void unsetRead()
            {
                while( true )
                {
                    EQASSERT( _state > _unlocked );
                    memoryBarrier();
                    const int32_t expected = _state;
                    if( _state.compareAndSwap( expected, expected-1 ))
                        return;
                }
            }

        /** 
         * Attempt to acquire the lock shared with other readers.
         *
         * @return true if the lock was set, false if
         *         it was not set.
         * @version 1.1.2
         */
        bool trySetRead()
            {
                memoryBarrier();
                const int32_t state = _state;
                // Note: 0 used here since using _unlocked unexplicably gives
                //       'undefined reference to co::base::SpinLock::_unlocked'
                const int32_t expected = (state==_writelocked) ? 0 : state;

                if( !_state.compareAndSwap( expected, expected+1 ))
                    return false;

                EQASSERTINFO( isSetRead(), _state << ", " << expected );
                return true;
            }

        /**
         * Test if the lock is set.
         * 
         * @return true if the lock is set, false if
         *         it is not set.
         * @version 1.0
         */
        bool isSet() { return ( _state != _unlocked ); }

        /**
         * Test if the lock is set exclusively.
         * 
         * @return true if the lock is set, false if it is not set.
         * @version 1.1.2
         */
        bool isSetWrite() { return ( _state == _writelocked ); }

        /**
         * Test if the lock is set shared.
         * 
         * @return true if the lock is set, false if it is not set.
         * @version 1.1.2
         */
        bool isSetRead() { return ( _state > _unlocked ); }

    private:
        a_int32_t _state;
    };
}

}
#endif //COBASE_SPINLOCK_H
