
/* Copyright (c) 2011 Daniel Pfeifer <daniel@pfeifer-mail.de>
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

#include "../uiFactory.h"

#include "../aglWindow.h"
#include "../aglPipe.h"
#include "../aglMessagePump.h"

namespace eq
{

static class : UIFactoryImpl< WINDOW_SYSTEM_AGL >
{
	eq::SystemWindow* _createSystemWindow(eq::Window* window) const
	{
		EQINFO << "Using AGLWindow" << std::endl;
		return new AGLWindow(window);
	}

	eq::SystemPipe* _createSystemPipe(eq::Pipe* pipe) const
	{
		EQINFO << "Using AGLPipe" << std::endl;
		return new AGLPipe(pipe);
	}

	eq::MessagePump* _createMessagePump() const
	{
		return new AGLMessagePump;
	}

	void _configInit(eq::Node* node) const
	{
#ifdef EQ_USE_MAGELLAN
		AGLEventHandler::initMagellan(node);
#endif
	}

	void _configExit(eq::Node* node) const
	{
#ifdef EQ_USE_MAGELLAN
		AGLEventHandler::exitMagellan(node);
#endif
	}
} _aglFactory;

} // namespace eq
