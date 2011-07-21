
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 * Copyright (c)      2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
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

namespace eq
{

static WindowSystemAbstract* _stack = 0;

WindowSystemAbstract::WindowSystemAbstract() :
		_next(_stack)
{
	_stack = this;
}

WindowSystem::WindowSystem() :
		_impl(_stack)
{
	EQASSERTINFO(_stack, "no window system available");
}

WindowSystem::WindowSystem(std::string const& type)
{
	EQASSERTINFO(_stack, "no window system available");

	for (WindowSystemAbstract* ws = _stack; ws; ws = ws->_next)
	{
		// TODO: maybe we should do case insesitive comparision?
		if (ws->name() == type)
		{
			this->_impl = ws;
			return;
		}
	}

	_impl = _stack;

    EQWARN << "Window system " << type << " not supported, "
           << "using " << _impl->name() << " instead."
           << std::endl;
}

bool WindowSystem::supports(std::string const& type)
{
	for (WindowSystemAbstract* ws = _stack; ws; ws = ws->_next)
	{
		// TODO: maybe we should do case insesitive comparision?
		if (ws->name() == type)
			return true;
	}

	return false;
}

void WindowSystem::configInit(Node* node)
{
	for (WindowSystemAbstract* ws = _stack; ws; ws = ws->_next)
	{
		ws->configInit(node);
	}
}

void WindowSystem::configExit(Node* node)
{
	for (WindowSystemAbstract* ws = _stack; ws; ws = ws->_next)
	{
		ws->configExit(node);
	}
}

std::string WindowSystem::name() const
{
	EQASSERT( _impl);
	return _impl->name();
}

SystemWindow* WindowSystem::createSystemWindow(Window* window) const
{
	EQASSERT( _impl);
	return _impl->createSystemWindow(window);
}

SystemPipe* WindowSystem::createSystemPipe(Pipe* pipe) const
{
	EQASSERT( _impl);
	return _impl->createSystemPipe(pipe);
}

MessagePump* WindowSystem::createMessagePump() const
{
	EQASSERT( _impl);
	return _impl->createMessagePump();
}

GPUInfos WindowSystem::discoverGPUs() const
{
	EQASSERT( _impl);
	return _impl->discoverGPUs();
}

bool WindowSystem::operator==(const WindowSystem& other) const
{
	return _impl == other._impl;
}

bool WindowSystem::operator!=(const WindowSystem& other) const
{
	return _impl != other._impl;
}

std::ostream& operator <<(std::ostream& os, const WindowSystem& ws)
{
	return os << ws.name();
}

} // namespace eq
