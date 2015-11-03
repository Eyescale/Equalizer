
/* Copyright (c) 2014-2015, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Stefan.Eilemann@epfl.ch
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

#define NOMINMAX // no min/max from windows.h
#pragma warning( disable: 4407 ) // see lunchbox/commandFunc.h
#include <eq/window.h> // be first to avoid max/min name clashes on Win32

#include "windowSystem.h"

#include "messagePump.h"
#include "pipe.h"
#include "shareContextWindow.h"
#include "window.h"
#include "windowImpl.h"
#include "windowFactory.h"

#include <eq/client.h>
#include <QApplication>
#include <QThread>

namespace eq
{
namespace qt
{

WindowSystem::WindowSystem()
    : _factory( new WindowFactory )
{
    qRegisterMetaType< WindowSettings >( "WindowSettings" );
    QCoreApplication* app = QApplication::instance();
    if( !app )
        return;

    _factory->moveToThread( app->thread( ));
    app->connect( this, SIGNAL( createImpl( const eq::Pipe*,
                                            const WindowSettings&, QThread* )),
                  _factory, SLOT( onCreateImpl( const eq::Pipe*,
                                                const WindowSettings&,
                                                QThread* )),
                  Qt::BlockingQueuedConnection );
}

WindowSystem::~WindowSystem()
{
    delete _factory;
}

std::string WindowSystem::getName() const
    { return QApplication::instance() ? "Qt" : ""; }

eq::SystemWindow* WindowSystem::createWindow( eq::Window* window,
                                              const WindowSettings& settings )
{
    LBDEBUG << "Using qt::Window" << std::endl;

    // QWindow creation/destruction must happen in the app thread; unblock main
    // thread to give QApplication the chance to process the createImpl signal.
    // Note that even a QOffscreenSurface is backed by a QWindow on some
    // platforms.
    window->getClient()->interruptMainThread();
    qt::detail::Window* impl = createImpl( window->getPipe(), settings,
                                           QThread::currentThread( ));
    Window* qtWindow = new Window( *window, settings, impl );
    qtWindow->connect( qtWindow, SIGNAL( destroyImpl( detail::Window* )),
                      _factory, SLOT( onDestroyImpl( detail::Window* )));
    return qtWindow;
}

eq::SystemPipe* WindowSystem::createPipe( eq::Pipe* pipe )
{
    return new Pipe( pipe );
}

eq::MessagePump* WindowSystem::createMessagePump()
{
    return new MessagePump;
}

bool WindowSystem::setupFont( util::ObjectManager& gl LB_UNUSED,
                              const void* key LB_UNUSED,
                              const std::string& name LB_UNUSED,
                              const uint32_t size LB_UNUSED ) const
{
    return false;
}

static WindowSystem _instance;

}
}
