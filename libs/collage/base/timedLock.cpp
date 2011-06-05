
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

#include "timedLock.h"

#include <co/base/debug.h>

namespace co
{
namespace base
{

TimedLock::TimedLock()
        : _locked( false )
{}

TimedLock::~TimedLock()
{}

bool TimedLock::set( const uint32_t timeout )
{
    _condition.lock();

    bool acquired = true;
    while( _locked )
    {
        if( !_condition.timedWait( timeout ))
        {
            acquired = false;
            break;
        }
    }

    if( acquired )
    {
        EQASSERT( !_locked );
        _locked = true;
    }

    _condition.unlock();
    return acquired;
}

void TimedLock::unset()
{
    _condition.lock();
    _locked = false;
    _condition.signal();
    _condition.unlock();
}


bool TimedLock::trySet()
{
    _condition.lock();
    
    bool acquired = false;
    if( _locked )
    {
        _locked  = true;
        acquired = true;
    }

    _condition.unlock();
    return acquired;
}

}
}
