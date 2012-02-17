
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "timedLock.h"

#include "condition.h"
#include "debug.h"

namespace co
{
namespace base
{
namespace detail
{
class TimedLock
{
public:
    TimedLock() : locked( false ) {}
    base::Condition condition;
    bool locked;
};
}

TimedLock::TimedLock()
        : _impl( new detail::TimedLock )
{}

TimedLock::~TimedLock()
{
    delete _impl;
}

bool TimedLock::set( const uint32_t timeout )
{
    _impl->condition.lock();

    bool acquired = true;
    while( _impl->locked )
    {
        if( !_impl->condition.timedWait( timeout ))
        {
            acquired = false;
            break;
        }
    }

    if( acquired )
    {
        EQASSERT( !_impl->locked );
        _impl->locked = true;
    }

    _impl->condition.unlock();
    return acquired;
}

void TimedLock::unset()
{
    _impl->condition.lock();
    _impl->locked = false;
    _impl->condition.signal();
    _impl->condition.unlock();
}


bool TimedLock::trySet()
{
    _impl->condition.lock();
    
    bool acquired = false;
    if( _impl->locked )
    {
        _impl->locked  = true;
        acquired = true;
    }

    _impl->condition.unlock();
    return acquired;
}

bool TimedLock::isSet()
{
    return _impl->locked;
}

}
}
