
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
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

#include "windowSystem.h"

#include "gl.h"

#include <eq/util/objectManager.h>
#include <eq/fabric/gpuInfo.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
static WindowSystemIF* _stack = 0;

WindowSystemIF::WindowSystemIF()
        : _next( _stack )
{
    _stack = this;
}

uint32_t WindowSystemIF::_setupLists( util::ObjectManager& gl, const void* key,
                                      const int num )
{
    GLuint lists = gl.getList( key );
    if( lists != util::ObjectManager::INVALID )
        gl.deleteList( key );

    if( num == 0 )
        lists = util::ObjectManager::INVALID;
    else
    {
        lists = gl.newList( key, num );
        LBASSERT( lists != util::ObjectManager::INVALID );
    }
    return lists;
}

WindowSystem::WindowSystem()
        : _impl( _stack )
{
    LBASSERTINFO( _stack, "no window system available" );
}

WindowSystem::WindowSystem( std::string const& type )
{
    _chooseImpl( type );
}

void WindowSystem::_chooseImpl( const std::string& name )
{
    LBASSERTINFO( _stack, "no window system available" );

    for( WindowSystemIF* ws = _stack; ws; ws = ws->_next )
    {
        // TODO: maybe we should do case insesitive comparision?
        if( ws->getName() == name )
        {
            _impl = ws;
            return;
        }
    }

    _impl = _stack;
    LBWARN << "Window system " << name << " not supported, " << "using "
           << _impl->getName() << " instead." << std::endl;
}

bool WindowSystem::supports( std::string const& type )
{
    for( WindowSystemIF* ws = _stack; ws; ws = ws->_next )
    {
        // TODO: maybe we should do case insensitive comparison?
        if( ws->getName() == type )
            return true;
    }

    return false;
}

void WindowSystem::configInit( Node* node )
{
    for( WindowSystemIF* ws = _stack; ws; ws = ws->_next )
        ws->configInit( node );
}

void WindowSystem::configExit( Node* node )
{
    for( WindowSystemIF* ws = _stack; ws; ws = ws->_next )
        ws->configExit(node );
}

std::string WindowSystem::getName() const
{
    LBASSERT( _impl );
    return _impl->getName();
}

SystemWindow* WindowSystem::createWindow( Window* window ) const
{
    LBASSERT( _impl );
    return _impl->createWindow(window );
}

SystemPipe* WindowSystem::createPipe( Pipe* pipe ) const
{
    LBASSERT( _impl );
    return _impl->createPipe(pipe );
}

MessagePump* WindowSystem::createMessagePump() const
{
    LBASSERT( _impl );
    return _impl->createMessagePump();
}

bool WindowSystem::setupFont( util::ObjectManager& gl, const void* key,
                              const std::string& name,
                              const uint32_t size ) const
{
    LBASSERT( _impl );
    return _impl->setupFont( gl, key, name, size );
}

bool WindowSystem::operator == ( const WindowSystem& other) const
{
    return _impl == other._impl;
}

bool WindowSystem::operator != ( const WindowSystem& other ) const
{
    return _impl != other._impl;
}

std::ostream& operator << ( std::ostream& os, const WindowSystem& ws )
{
    return os << ws.getName();
}

co::DataOStream& operator << ( co::DataOStream& os, const WindowSystem& ws )
{
    return os << ws.getName();
}

co::DataIStream& operator >> ( co::DataIStream& is, WindowSystem& ws )
{
    std::string name;
    is >> name;
    ws = WindowSystem( name );
    return is;
}

} // namespace eq
