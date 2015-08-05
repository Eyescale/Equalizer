
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

#include "window.h"
#include "windowImpl.h"
#include "windowEvent.h"

namespace eq
{
namespace qt
{

detail::Window* Window::createImpl( const WindowSettings& settings,
                                    QOpenGLContext* sharedContext,
                                    QThread* thread )
{
    const int32_t drawable = getAttribute( IATTR_HINT_DRAWABLE );
    detail::Window* window = 0;
    if( drawable == eq::WINDOW )
        window = new detail::QWindowWrapper( settings, sharedContext );
    else
        window = new detail::QOffscreenSurfaceWrapper( settings, sharedContext);

    window->getContext()->moveToThread( thread );
    return window;
}

Window::Window( NotifierInterface& parent_, const WindowSettings& settings,
                detail::Window* impl )
    : WindowIF( parent_, settings )
    , _impl( impl )
{
}

Window::~Window()
{
    destroyImpl( _impl );
}

bool Window::configInit()
{
    if( !_impl->configInit( *this ))
        return false;

    makeCurrent();
    initGLEW();

    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == FBO )
        return configInitFBO();
    return true;
}

void Window::configExit()
{
    configExitFBO();
    makeCurrent();
    exitGLEW();
    _impl->doneCurrent();
    _impl->configExit();
}

QOpenGLContext* Window::getContext() const
{
    return _impl->getContext();
}

void Window::makeCurrent( const bool cache LB_UNUSED ) const
{
    // Qt (at least on Windows) complains about call to non-current context
    // while swapBuffers()
#ifndef _MSC_VER
    if( cache && isCurrent( ))
        return;
#endif

    _impl->makeCurrent(); // Make real GL context current first
    WindowIF::makeCurrent(); // Validate FBO binding and caching state
}

void Window::doneCurrent() const
{
    if( !isCurrent( ))
        return;

    _impl->doneCurrent();
    WindowIF::doneCurrent();
}

void Window::swapBuffers()
{
    _impl->swapBuffers();
}

void Window::joinNVSwapBarrier( const uint32_t /*group*/,
                                const uint32_t /*barrier*/ )
{
}

void Window::leaveNVSwapBarrier()
{
}

bool Window::processEvent( const WindowEvent& event_ )
{
    // Resizing the FBO if needed
    if( getFrameBufferObject() &&
        event_.eq::Event::type == eq::Event::WINDOW_RESIZE )
    {
        getFrameBufferObject()->resize( event_.resize.w, event_.resize.h );
    }
    return SystemWindow::processEvent( event_ );
}

QObject* Window::getEventProcessor()
{
    return _impl->getEventProcessor();
}

}
}
