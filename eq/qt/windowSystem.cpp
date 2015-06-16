
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *               2015, Juan Hernando <jhernando@fi.upm.es>
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

#include "messagePump.h"
#include "windowSystem.h"
#include "window.h"
#include "shareContextWindow.h"

#include <eq/client.h>
#include <QApplication>
#include <QThread>

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

QOpenGLContext* _getShareContext( const WindowSettings& settings )
{
    const SystemWindow* shareWindow = settings.getSharedContextWindow();
    const Window* window = dynamic_cast< const Window* >( shareWindow );
    if( window )
        // This only works if configInit has already been called in the window
        return window->getContext();

    const ShareContextWindow* dummyWindow =
        dynamic_cast< const ShareContextWindow* >( shareWindow );
    if( dummyWindow )
        return dummyWindow->getContext();

    return 0;
}

}

WindowSystem::WindowSystem()
{
    qRegisterMetaType< WindowSettings >( "WindowSettings" );
}

WindowSystem::~WindowSystem()
{
}

std::string WindowSystem::getName() const
    { return QApplication::instance() ? "Qt" : ""; }

eq::SystemWindow* WindowSystem::createWindow( eq::Window* window,
                                              const WindowSettings& settings )
{
    if( _useSystemWindowSystem( settings, window->getSharedContextWindow()))
        return getSystemWindowSystem().createWindow( window, settings );

    QOpenGLContext* context = _getShareContext( settings );

    LBINFO << "Using qt::Window" << std::endl;

    // The following statement is not cross-platform.
    // In the onscreen window case, a QWindow is used. According to the Qt
    // documentation, some platforms require this object be created in the main
    // GUI thread, however it doesn't tell which are those platforms. Since we
    // don't care about mobile OSes, we are happy if this works on Window, Mac
    // and Linux.
    return new Window( *window, settings, context );
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
    // pbuffer is not (yet) supported; OFF is used for transfer window
    // which can also not be emulated yet (maybe with QGLContext only)
    const int32_t settingsDrawable = getAttribute( IATTR_HINT_DRAWABLE );
    if( settingsDrawable == eq::PBUFFER || settingsDrawable == eq::OFF )
        return true;

    if( !sharedWindow )
        return false;

    const int32_t windowDrawable = sharedWindow->getSettings().getIAttribute(
                                          WindowSettings::IATTR_HINT_DRAWABLE );
    return windowDrawable == eq::PBUFFER || windowDrawable == eq::OFF;
}

}
}
