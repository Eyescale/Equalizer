
/* Copyright (c) 2007-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Pfeifer <daniel@pfeifer-mail.de>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
namespace
{
std::vector< WindowSystemImpl >& _getRegistry()
{
    static std::vector< WindowSystemImpl > registry;
    return registry;
}
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

WindowSystem::WindowSystem( const std::string& type )
    : _impl( _chooseImpl( type ))
{
    LBINFO << "Using " << _impl->getName() << " window system" << std::endl;
}

void WindowSystem::add( WindowSystemImpl impl )
{
    _getRegistry().push_back( impl );
}

void WindowSystem::clear()
{
    _getRegistry().clear();
}

WindowSystemImpl WindowSystem::_chooseImpl( const std::string& name )
{
    LBASSERTINFO( !_getRegistry().empty(), "no window system available" );

    for( auto ws : _getRegistry( ))
        if( ws->getName() == name )
            return ws;

    for( auto ws : _getRegistry( ))
        if( !ws->getName().empty( ))
            return ws;

    return _getRegistry().back();
}

bool WindowSystem::supports( std::string const& type )
{
    for( auto ws : _getRegistry() )
        if( ws->getName() == type )
            return true;

    return false;
}

void WindowSystem::configInit( Node* node )
{
    for( auto ws : _getRegistry() )
        ws->configInit( node );
}

void WindowSystem::configExit( Node* node )
{
    for( auto ws : _getRegistry() )
        ws->configExit(node );
}

std::string WindowSystem::getName() const
{
    LBASSERT( _impl );
    return _impl->getName();
}

SystemWindow* WindowSystem::createWindow( Window* window,
                                          const WindowSettings& settings )
{
    LBASSERT( _impl );
    return _impl->createWindow( window, settings );
}

SystemPipe* WindowSystem::createPipe( Pipe* pipe )
{
    LBASSERT( _impl );
    return _impl->createPipe( pipe );
}

MessagePump* WindowSystem::createMessagePump()
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

bool WindowSystem::hasMainThreadEvents() const
{
    LBASSERT( _impl );
    return _impl->hasMainThreadEvents();
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
