
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

#include "eventHandler.h"

#include "window.h"
#include "windowEvent.h"


namespace eq
{
namespace qt
{

EventHandler::EventHandler( WindowIF& window )
    : _window( window )
{
}

EventHandler::~EventHandler()
{
}

bool EventHandler::event( QEvent* evt )
{
    if( evt->type() != QEvent::User )
    {
        LBUNREACHABLE;
        return false;
    }

    WindowEvent& windowEvent  = static_cast< WindowEvent& >( *evt );
    switch( windowEvent.eq::Event::type )
    {
    case Event::WINDOW_POINTER_MOTION:
    case Event::WINDOW_POINTER_BUTTON_PRESS:
    case Event::WINDOW_POINTER_BUTTON_RELEASE:
        _computePointerDelta( windowEvent );
        break;
    default:
        break;
    }

    return _window.processEvent( windowEvent );
}

}
}
