
/* Copyright (c) 2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "uiFactory.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{

namespace
{
UIFactory* _stack = 0;
}

UIFactory::UIFactory( eq::WindowSystem type )
        : _type(type)
        , _next( _stack )
{
    _stack = this;
}


eq::SystemWindow* UIFactory::createSystemWindow( eq::WindowSystem type,
                                                 eq::Window* window )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        if (factory->_type == type)
            return factory->_createSystemWindow(window);
    }

    //EQUNREACHABLE;
    EQASSERT(!"unreachable");
    return 0;
}

eq::SystemPipe* UIFactory::createSystemPipe( eq::WindowSystem type,
                                             eq::Pipe* pipe )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        if (factory->_type == type)
            return factory->_createSystemPipe(pipe);
    }

    //EQUNREACHABLE;
    EQASSERT(!"unreachable");
    return 0;
}

eq::MessagePump* UIFactory::createMessagePump( eq::WindowSystem type )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        if (factory->_type == type)
            return factory->_createMessagePump();
    }

    //EQUNREACHABLE;
    EQASSERT(!"unreachable");
    return 0;
}

GPUInfos UIFactory::discoverGPUs()
{
    if( _stack )
        return _stack->_discoverGPUs();

    EQASSERT(!"unreachable");
    GPUInfos result;
    return result;
}


void UIFactory::configInit( eq::Node* node )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        factory->_configInit(node);
    }
}

void UIFactory::configExit( eq::Node* node )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        factory->_configExit(node);
    }
}

bool UIFactory::supports( eq::WindowSystem type )
{
    for (UIFactory* factory = _stack; factory; factory = factory->_next)
    {
        if (factory->_type == type)
            return true;
    }

    return false;
}

} // namespace eq
