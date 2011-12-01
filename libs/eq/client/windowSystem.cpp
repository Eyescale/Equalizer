
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

WindowSystem::WindowSystem()
        : _impl( _stack )
{
    EQASSERTINFO( _stack, "no window system available" );
}

WindowSystem::WindowSystem( std::string const& type )
{
    _chooseImpl( type );
}

#ifndef EQ_2_0_API
static std::string _getName( const WindowSystemEnum type )
{
    switch( type )
    {
      case WINDOW_SYSTEM_AGL:
          return "AGL";

      case WINDOW_SYSTEM_GLX:
          return "GLX";
         
      case WINDOW_SYSTEM_WGL:
          return "WGL";

      default:
          return "";
    }
}

WindowSystem::WindowSystem( const WindowSystemEnum type )
{
    _chooseImpl( _getName( type ));
}

bool WindowSystem::operator == ( const WindowSystemEnum other) const
{
    EQASSERT( _impl );
    return _impl->getName() == _getName( other );
}

bool WindowSystem::operator != ( const WindowSystemEnum other ) const
{
    EQASSERT( _impl );
    return _impl->getName() != _getName( other );
}

WindowSystem::operator WindowSystemEnum() const
{
    EQASSERT( _impl );
    if( _impl->getName() == "AGL" )
        return WINDOW_SYSTEM_AGL;
    if( _impl->getName() == "GLX" )
        return WINDOW_SYSTEM_GLX;
    if( _impl->getName() == "WGL" )
        return WINDOW_SYSTEM_WGL;
    return WINDOW_SYSTEM_NONE;
}
#endif

void WindowSystem::_chooseImpl( const std::string& name )
{
    EQASSERTINFO( _stack, "no window system available" );

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
    EQWARN << "Window system " << name << " not supported, " << "using "
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
    if( _stack )
        _stack->configInit( node );
}

void WindowSystem::configExit( Node* node )
{
    if( _stack )
        _stack->configExit(node );
}

std::string WindowSystem::getName() const
{
    EQASSERT( _impl );
    return _impl->getName();
}

SystemWindow* WindowSystem::createWindow( Window* window ) const
{
    EQASSERT( _impl );
    return _impl->createWindow(window );
}

SystemPipe* WindowSystem::createPipe( Pipe* pipe ) const
{
    EQASSERT( _impl );
    return _impl->createPipe(pipe );
}

MessagePump* WindowSystem::createMessagePump() const
{
    EQASSERT( _impl );
    return _impl->createMessagePump();
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
