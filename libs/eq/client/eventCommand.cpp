
/* Copyright (c) 2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "eventCommand.h"

#include "event.h"

namespace eq
{

namespace detail
{
class EventCommand
{
public:
    EventCommand() {}

    uint32_t eventType;
};
}

EventCommand::EventCommand( const co::Command& command )
    : co::ObjectCommand( command )
    , _impl( new detail::EventCommand )
{
    _init();
}

EventCommand::EventCommand( const EventCommand& rhs )
    : co::ObjectCommand( rhs )
    , _impl( new detail::EventCommand )
{
    _init();
}

EventCommand::~EventCommand()
{
    delete _impl;
}

void EventCommand::_init()
{
    if( isValid( ))
        *this >> _impl->eventType;
}

uint32_t EventCommand::getEventType() const
{
    return _impl->eventType;
}

std::ostream& operator << ( std::ostream& os, const EventCommand& event )
{
    os << "Event command, event type " << event.getEventType();
    return os;
}

}
