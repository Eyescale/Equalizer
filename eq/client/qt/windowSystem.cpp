
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

#define NOMINMAX // no min/max from windows.h
#pragma warning( disable: 4407 ) // see lunchbox/commandFunc.h
#include <eq/client/window.h> // be first to avoid max/min name clashes on Win32

#include "windowSystem.h"

#include "glWidget.h"
#include "messagePump.h"
#include "widgetFactory.h"
#include "window.h"

#include <boost/bind.hpp>
#include <eq/client/client.h>
#include <QApplication>
#include <QThread>


static eq::qt::WindowSystem _qtFactory;

namespace eq
{
namespace qt
{
namespace
{
eq::WindowSystem getSystemWindowSystem()
{
#ifdef AGL
    return eq::WindowSystem( "AGL" );
#elif GLX
    return eq::WindowSystem( "GLX" );
#elif WGL
    return eq::WindowSystem( "WGL" );
#endif
}
}

WindowSystem::WindowSystem()
    : QObject()
    , _factory( 0 )
{
    qRegisterMetaType< WindowSettings >( "WindowSettings" );
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
    if( _useSystemWindowSystem( settings, window->getSharedContextWindow()))
        return getSystemWindowSystem().createWindow( window, settings );

    LBINFO << "Using qt::Window" << std::endl;

    if( !_factory )
        _setupFactory();

    window->getClient()->interruptMainThread();
    GLWidget* widget = createWidget( window, settings,
                                     QThread::currentThread( ));

    return new Window( *window, settings, widget,
                       boost::bind( &WindowSystem::destroyWidget, this, _1 ));
}

eq::SystemPipe* WindowSystem::createPipe( eq::Pipe* pipe )
{
    return getSystemWindowSystem().createPipe( pipe );
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
#ifdef AGL
    return false;
#else
    return getSystemWindowSystem().setupFont( gl, key, name, size );
#endif
}

#define getAttribute( attr ) settings.getIAttribute( WindowSettings::attr )

bool WindowSystem::_useSystemWindowSystem( const WindowSettings& settings,
                                           const eq::Window* sharedWindow )
{
    return getAttribute( IATTR_HINT_DRAWABLE ) == eq::PBUFFER ||
           (sharedWindow && sharedWindow->getSettings().getIAttribute(
                      WindowSettings::IATTR_HINT_DRAWABLE ) == eq::PBUFFER );
}

void WindowSystem::_setupFactory()
{
    QCoreApplication* app = QApplication::instance();
    _factory = new WidgetFactory;
    _factory->moveToThread( app->thread( ));

    app->connect( this, SIGNAL(createWidget( eq::Window*, const WindowSettings&,
                                             QThread* )),
                  _factory, SLOT(onCreateWidget( eq::Window*,
                                                 const WindowSettings&,
                                                 QThread* )),
                  Qt::BlockingQueuedConnection );

    app->connect( this, SIGNAL(destroyWidget( GLWidget* )),
                  _factory, SLOT(onDestroyWidget( GLWidget* )));
}

void WindowSystem::_deleteGLWidget( GLWidget* widget )
{
    destroyWidget( widget );
}

}
}
