
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

#ifndef EQ_UI_FACTORY_H
#define EQ_UI_FACTORY_H

#include <eq/window.h>
#include <eq/pipe.h>
#include <eq/node.h>

namespace eq
{

class UIFactory
{
public:
    static bool supports( eq::WindowSystem type );

    static SystemWindow* createSystemWindow( eq::WindowSystem type,
                                             eq::Window* window );

    static SystemPipe* createSystemPipe( eq::WindowSystem type, eq::Pipe* pipe);

    static MessagePump* createMessagePump( eq::WindowSystem type );

    static void configInit( eq::Node* node );
    static void configExit( eq::Node* node );

protected:
    UIFactory( eq::WindowSystem type );
    virtual ~UIFactory() {}

private:
    virtual eq::SystemWindow* _createSystemWindow(eq::Window* window) const = 0;

    virtual eq::SystemPipe* _createSystemPipe(eq::Pipe* pipe) const = 0;

    virtual eq::MessagePump* _createMessagePump() const = 0;

    virtual void _configInit(eq::Node* node) const {}
    virtual void _configExit(eq::Node* node) const {}

private:
    eq::WindowSystem _type;
    UIFactory* _next;
};

template<  eq::WindowSystem WindowSystem > struct UIFactoryImpl: UIFactory
{
    UIFactoryImpl() : UIFactory( WindowSystem ) {}
};

} // namespace eq

#endif /* EQ_UI_FACTORY_H */
