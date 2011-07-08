
/* Copyright (c) 2011 Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011 Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/types.h>
#include <eq/windowSystem.h>

namespace eq
{
class UIFactory
{
public:
    static bool supports( WindowSystem type );

    static SystemWindow* createSystemWindow( WindowSystem type,
                                             Window* window );

    static SystemPipe* createSystemPipe( WindowSystem type, Pipe* pipe);

    static MessagePump* createMessagePump( WindowSystem type );

    EQ_API static GPUInfos discoverGPUs();

    static void configInit( Node* node );
    static void configExit( Node* node );

protected:
    UIFactory( WindowSystem type );
    virtual ~UIFactory() {}

private:
    virtual SystemWindow* _createSystemWindow( Window* window ) const = 0;
    virtual SystemPipe* _createSystemPipe( Pipe* pipe ) const = 0;
    virtual MessagePump* _createMessagePump() const = 0;
    virtual GPUInfos _discoverGPUs() const = 0;

    virtual void _configInit( Node* node ) const {}
    virtual void _configExit( Node* node ) const {}

private:
    WindowSystem _type;
    UIFactory* _next;
};

template< WindowSystem WindowSystem > struct UIFactoryImpl : UIFactory
{
    UIFactoryImpl() : UIFactory( WindowSystem ) {}
};

} // namespace eq

#endif /* EQ_UI_FACTORY_H */
