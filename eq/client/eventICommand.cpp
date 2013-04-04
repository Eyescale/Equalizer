
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

#include "eventICommand.h"

#ifndef EQ_2_0_API
#include "eq/fabric/commands.h"
#include "configEvent.h"
#endif
#include "event.h"

namespace eq
{

namespace detail
{
class EventICommand
{
public:
    EventICommand() {}

    uint32_t eventType;
};
}

EventICommand::EventICommand( const co::ICommand& command )
    : co::ObjectICommand( command )
    , _impl( new detail::EventICommand )
{
    _init();
}

EventICommand::EventICommand( const EventICommand& rhs )
    : co::ObjectICommand( rhs )
    , _impl( new detail::EventICommand )
{
    _init();
}

EventICommand::~EventICommand()
{
    delete _impl;
}

void EventICommand::_init()
{
    if( isValid( ))
    {
#ifndef EQ_2_0_API
        if (getCommand() == fabric::CMD_CONFIG_EVENT_OLD)
        {
            co::ObjectICommand copy = *this;
            uint64_t size = copy.get<uint64_t>();
            ConfigEvent event;
            copy >> co::Array<void>(&event, size);
            _impl->eventType = event.data.type;
            return;
        }
#endif
        *this >> _impl->eventType;
    }
}

uint32_t EventICommand::getEventType() const
{
    return _impl->eventType;
}

const eq::Event &EventICommand::convertToEvent()
{
#ifndef EQ_2_0_API
    if (getCommand() == fabric::CMD_CONFIG_EVENT_OLD)
    {
        const uint64_t size = get< uint64_t >();
        return reinterpret_cast< const ConfigEvent* >
            ( getRemainingBuffer( size ) )->data;
    }
#endif
    return *reinterpret_cast< const eq::Event* >
        ( getRemainingBuffer( sizeof(eq::Event) ) );
}

std::ostream& operator << ( std::ostream& os, const EventICommand& event )
{
    os << "Event command, event type " << Event::Type( event.getEventType( ));
    return os;
}

}
