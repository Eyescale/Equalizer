
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
                      2009, Maxim Makhinya
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

#include "global.h"
#include "pipe.h"

namespace eq
{

SystemWindow::SystemWindow( Window* parent )
    : _window( parent )
{
    LBASSERT( _window ); 
}

SystemWindow::~SystemWindow()
{
}

const Pipe* SystemWindow::getPipe() const
{
    LBASSERT( _window );
    return _window->getPipe();
}
Pipe* SystemWindow::getPipe()
{
    LBASSERT( _window );
    return _window->getPipe();
}

const Node* SystemWindow::getNode() const
{
    LBASSERT( _window );
    return _window->getNode();
}
Node* SystemWindow::getNode()
{
    LBASSERT( _window );
    return _window->getNode();
}

const Config* SystemWindow::getConfig() const
{
    LBASSERT( _window );
    return _window->getConfig();
}
Config* SystemWindow::getConfig()
{
    LBASSERT( _window );
    return _window->getConfig();
}

int32_t SystemWindow::getIAttribute( const Window::IAttribute attr ) const
{
    LBASSERT( _window );
    return _window->getIAttribute( attr );
}

void SystemWindow::setError( const int32_t error )
{
    LBASSERT( _window );
    _window->setError( error );
}

eq::fabric::Error SystemWindow::getError() const
{
    LBASSERT( _window );
    return _window->getError();
}

bool SystemWindow::processEvent( const Event& event )
{
    LBASSERT( _window );
    return _window->processEvent( event );
}

}
