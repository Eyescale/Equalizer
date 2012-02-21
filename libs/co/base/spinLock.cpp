
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include "spinLock.h"
#include <co/base/atomic.h>
#include <co/base/thread.h>

namespace co
{
namespace base
{
namespace
{
static const long _writelocked = -1;
static const long _unlocked = 0;
}
namespace detail
{
class SpinLock
{
public:
    SpinLock() : _state( _unlocked ) {}
    ~SpinLock() { _state = _unlocked; }

    inline void set()
        {
            while( true )
            {
                if( trySet( ))
                    return;
                base::Thread::yield();
            }
        }

    inline void unset()
        {
            EQASSERT( _state == _writelocked );
            _state = _unlocked;
        }

    inline bool trySet()
        {
            if( !_state.compareAndSwap( _unlocked, _writelocked ))
                return false;
            EQASSERTINFO( isSetWrite(), _state );
            return true;
        }

    inline void setRead()
        {
            while( true )
            {
                if( trySetRead( ))
                    return;
                base::Thread::yield();
            }
        }

    inline void unsetRead()
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

    inline bool trySetRead()
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

    inline bool isSet() { return ( _state != _unlocked ); }
    inline bool isSetWrite() { return ( _state == _writelocked ); }
    inline bool isSetRead() { return ( _state > _unlocked ); }

private:
    a_int32_t _state;
};
}

SpinLock::SpinLock()
        : _impl( new detail::SpinLock ) {}

SpinLock::~SpinLock()
{
    delete _impl;
}

void SpinLock::set()
{
    _impl->set();
}

void SpinLock::unset()
{
    _impl->unset();
}

bool SpinLock::trySet()
{
    return _impl->trySet();
}

void SpinLock::setRead()
{
    _impl->setRead();
}

void SpinLock::unsetRead()
{
    _impl->unsetRead();
}

bool SpinLock::trySetRead()
{
    return _impl->trySetRead();
}

bool SpinLock::isSet()
{
    return _impl->isSet();
}

bool SpinLock::isSetWrite()
{
    return _impl->isSetWrite();
}

bool SpinLock::isSetRead()
{
    return _impl->isSetRead();
}

}
}
