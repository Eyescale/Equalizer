
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "messagePump.h"

#include <lunchbox/clock.h>
#include <lunchbox/sleep.h>
#include <QEventLoop>

namespace eq
{
namespace qt
{
MessagePump::MessagePump()
{
}

MessagePump::~MessagePump()
{
}

void MessagePump::postWakeup()
{
    _wakeup = 1;
}

void MessagePump::dispatchOne( const uint32_t timeout )
{
    // dispatchOne / wakeup semantics are not implementable. poll.
    // * QEventLoop::wakeup does not wakup processEvents( WaitForMoreEvents )
    // * processEvents( timeout ) returns as soon as there are no events

    QEventLoop eventLoop;
    lunchbox::Clock _clock;
    if( eventLoop.processEvents( QEventLoop::AllEvents ))
        return;

    uint32_t timeLeft = std::max( 0u, timeout - uint32_t( _clock.getTimef( )));
    while( timeLeft )
    {
        if( _wakeup == 1 )
        {
            _wakeup = 0;
            return;
        }
        lunchbox::sleep( std::min( 10u, timeLeft ));
        if( eventLoop.processEvents( QEventLoop::AllEvents ))
            return;
        timeLeft = std::max( 0u, timeout - uint32_t( _clock.getTimef( )));
    }
}

void MessagePump::dispatchAll()
{
    QEventLoop eventLoop;
    eventLoop.processEvents();
}

}
}
