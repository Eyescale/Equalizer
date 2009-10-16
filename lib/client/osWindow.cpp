
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
                          , Maxim Makhinya
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

#include "osWindow.h"

#include "frameBufferObject.h"
#include "global.h"
#include "pipe.h"

using namespace std;

namespace eq
{

OSWindow::OSWindow( Window* parent )
    : _window( parent )
{
    EQASSERT( _window ); 
}

OSWindow::~OSWindow()
{
}

const Pipe* OSWindow::getPipe() const
{
    EQASSERT( _window );
    return _window->getPipe();
}
Pipe* OSWindow::getPipe()
{
    EQASSERT( _window );
    return _window->getPipe();
}

const Node* OSWindow::getNode() const
{
    EQASSERT( _window );
    return _window->getNode();
}
Node* OSWindow::getNode()
{
    EQASSERT( _window );
    return _window->getNode();
}

const Config* OSWindow::getConfig() const
{
    EQASSERT( _window );
    return _window->getConfig();
}
Config* OSWindow::getConfig()
{
    EQASSERT( _window );
    return _window->getConfig();
}

int32_t OSWindow::getIAttribute( const Window::IAttribute attr ) const
{
    EQASSERT( _window );
    return _window->getIAttribute( attr );
}

}
