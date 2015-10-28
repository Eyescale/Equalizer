
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQ_QT_WINDOWSYSTEM_H
#define EQ_QT_WINDOWSYSTEM_H

#include "types.h"
#include <eq/windowSystem.h> // base class
#include <QObject>

namespace eq
{
namespace qt
{
namespace detail { class Window; }

class WindowSystem : public QObject, public WindowSystemIF
{
    Q_OBJECT

public:
    EQ_API WindowSystem();
    EQ_API ~WindowSystem();

signals:
    eq::qt::detail::Window* createImpl( const eq::Pipe*, const WindowSettings&,
                                        QThread* );

private:
    WindowFactory* _factory;

    std::string getName() const final;
    eq::SystemWindow* createWindow( eq::Window* window,
                                    const WindowSettings& settings ) final;
    eq::SystemPipe* createPipe( eq::Pipe* pipe ) final;
    eq::MessagePump* createMessagePump() final;
    bool hasMainThreadEvents() const final { return true; }
    bool setupFont( util::ObjectManager& gl, const void* key,
                    const std::string& name, const uint32_t size ) const final;
};

}
}

#endif
