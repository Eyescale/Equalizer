
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
    Window( QGLWidget* glWidget_,
            eq::qt::Window::DeleteGLWidgetFunc deleteGLWidget_ )
        : glWidget( glWidget_ )
        , deleteGLWidget( deleteGLWidget_ )
    {}

    ~Window()
    {
        if( deleteGLWidget )
            deleteGLWidget( glWidget );
        else
            delete glWidget;
    }

    QGLWidget* glWidget;
    const eq::qt::Window::DeleteGLWidgetFunc deleteGLWidget;
};
}

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                QGLWidget* glWidget, DeleteGLWidgetFunc deleteGLWidget )
    : WindowIF( parent, settings )
    , _impl( new detail::Window( glWidget, deleteGLWidget ))
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

    if( getGLWidget( ))
    {
        getGLWidget()->setParent( this );
        initEventHandler();
    }

    makeCurrent();
    initGLEW();

    bool success = true;
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == FBO )
        success = configInitFBO();

    return success && _impl->glWidget->isValid();
}

void Window::configExit()
{
    configExitFBO();
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

QGLWidget* Window::getQGLWidget() const
{
    return _impl->glWidget;
}

GLWidget* Window::getGLWidget() const
{
    return dynamic_cast< GLWidget* >( _impl->glWidget );
}

bool Window::processEvent( const WindowEvent& event )
{
    return SystemWindow::processEvent( event );
}

void Window::initEventHandler()
{
    if( getGLWidget( ))
        getGLWidget()->initEventHandler();
}

void Window::exitEventHandler()
{
    if( getGLWidget( ))
        getGLWidget()->exitEventHandler();
}

}
}
