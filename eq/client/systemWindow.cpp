
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2009, Maxim Makhinya
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

#include "systemWindow.h"

#include "notifierInterface.h"

#include <co/objectOCommand.h>

namespace eq
{

SystemWindow::SystemWindow( NotifierInterface& parent,
                            const WindowSettings& settings )
    : _parent( parent )
    , _settings( settings )
{
}

SystemWindow::~SystemWindow()
{
}

void SystemWindow::setPixelViewport( const PixelViewport& pvp )
{
    _settings.setPixelViewport( pvp );
}

const PixelViewport& SystemWindow::getPixelViewport() const
{
    return _settings.getPixelViewport();
}

uint32_t SystemWindow::getColorFormat() const
{
    return _settings.getColorFormat();
}

void SystemWindow::setName( const std::string& name )
{
    _settings.setName( name );
}

const std::string& SystemWindow::getName() const
{
    return _settings.getName();
}

int32_t
SystemWindow::getIAttribute( const WindowSettings::IAttribute attr ) const
{
    return _settings.getIAttribute( attr );
}

const SystemWindow* SystemWindow::getSharedContextWindow() const
{
    return _settings.getSharedContextWindow();
}

EventOCommand SystemWindow::sendError( const uint32_t error )
{
    return _parent.sendError( error );
}

bool SystemWindow::processEvent( const Event& event )
{
    return _parent.processEvent( event );
}

}
