
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/client/api.h>
#include <eq/client/types.h>

namespace eq
{

class WindowSystemAbstract;

/** The list of possible window systems. @sa Pipe::getWindowSystem() */
class WindowSystem
{
public:
	WindowSystem();
	WindowSystem(std::string const& type);

	static bool supports(std::string const& type);

	static void configInit(Node* node);
	static void configExit(Node* node);

	std::string name() const;

	SystemWindow* createSystemWindow(Window* window) const;
	SystemPipe* createSystemPipe(Pipe* pipe) const;
	MessagePump* createMessagePump() const;
	GPUInfos discoverGPUs() const;

	bool operator==(const WindowSystem& other) const;
	bool operator!=(const WindowSystem& other) const;

	friend std::ostream& operator <<(std::ostream& os, const WindowSystem&);

private:
	const WindowSystemAbstract* _impl;
};

/** Print the window system name to the given output stream. @version 1.0 */
EQ_API std::ostream& operator <<(std::ostream& os, const WindowSystem&);


/** This is the extension point for new Windowing Toolkits */
class WindowSystemAbstract
{
protected:
	WindowSystemAbstract();
	virtual ~WindowSystemAbstract() {}

private:
	virtual std::string name() const = 0;

	virtual SystemWindow* createSystemWindow(Window* window) const = 0;
	virtual SystemPipe* createSystemPipe(Pipe* pipe) const = 0;
	virtual MessagePump* createMessagePump() const = 0;
	virtual GPUInfos discoverGPUs() const = 0;

	virtual void configInit(Node* node) const {}
	virtual void configExit(Node* node) const {}

private:
	WindowSystemAbstract* _next;
	friend class WindowSystem;
};

template< char A, char B, char C = '\0', char D = '\0' >
class WindowSystemImpl : WindowSystemAbstract
{
	std::string name() const
	{
		return _name;
	}

	static const char _name[];
};

template< char A, char B, char C, char D >
const char WindowSystemImpl< A, B, C, D >::_name[] = { A, B, C, D, '\0' };

} // namespace eq

#endif // EQ_WINDOWSYSTEM_H
