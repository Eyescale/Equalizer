
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

#include "window.h"

#include "glWidget.h"
#include "windowEvent.h"

namespace eq
{
namespace qt
{
namespace detail
{

class Window
{
public:
    Window( GLWidget* glWidget_ )
        : glWidget( glWidget_ )
    {}

    GLWidget* glWidget;
};
}

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                GLWidget* glWidget )
    : WindowIF( parent, settings )
    , _impl( new detail::Window( glWidget ))
{
}

Window::~Window()
{
    delete _impl;
}

bool Window::configInit()
{
    PixelViewport pvp;
    pvp.x = _impl->glWidget->x();
    pvp.y = _impl->glWidget->y();
    pvp.w = _impl->glWidget->width();
    pvp.h = _impl->glWidget->height();
    setPixelViewport( pvp );

    _impl->glWidget->setParent( this );
    initEventHandler();

    makeCurrent();
    initGLEW();

    return _impl->glWidget->isValid();
}

void Window::configExit()
{
    makeCurrent();
    exitGLEW();

    exitEventHandler();
    _impl->glWidget->doneCurrent();
}

void Window::makeCurrent( const bool /*cache*/ ) const
{
    _impl->glWidget->makeCurrent();
}

void Window::swapBuffers()
{
    _impl->glWidget->swapBuffers();
}

void Window::joinNVSwapBarrier( const uint32_t /*group*/,
                                const uint32_t /*barrier*/ )
{
}

void Window::leaveNVSwapBarrier()
{
}

GLWidget* Window::getGLWidget() const
{
    return _impl->glWidget;
}

bool Window::processEvent( const WindowEvent& event )
{
    return SystemWindow::processEvent( event );
}

void Window::initEventHandler()
{
    _impl->glWidget->initEventHandler();
}

void Window::exitEventHandler()
{
    _impl->glWidget->exitEventHandler();
}

}
}
