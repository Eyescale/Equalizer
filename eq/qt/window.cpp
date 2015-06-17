
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

#include "window.h"

#include "windowEvent.h"
#include "eventHandler.h"

#include <eq/util/frameBufferObject.h>
#include <lunchbox/monitor.h>

#include <QEventLoop>
#include <QExposeEvent>
#include <QOpenGLContext>
#include <QPointer>
#include <QOffscreenSurface>
#include <QWindow>

namespace eq
{
namespace qt
{
namespace detail
{

#define getAttribute( attr ) settings.getIAttribute( WindowSettings::attr )

namespace
{
QSurfaceFormat _createFormat( const WindowSettings& settings )
{
    // defaults: http://doc.qt.io/qt-5/qsurfaceformat.html
    QSurfaceFormat format;

    const int coreProfile = getAttribute( IATTR_HINT_CORE_PROFILE );
    if( coreProfile == ON )
    {
        format.setMajorVersion( getAttribute( IATTR_HINT_OPENGL_MAJOR ));
        format.setMinorVersion( getAttribute( IATTR_HINT_OPENGL_MINOR ));
        format.setProfile( QSurfaceFormat::CoreProfile );
    }

    const int colorSize = getAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == eq::AUTO )
    {
        const int colorBits = ( colorSize > 0 ? colorSize : 8 );
        format.setRedBufferSize( colorBits );
        format.setGreenBufferSize( colorBits );
        format.setBlueBufferSize( colorBits );
    }

    const int alphaPlanes = getAttribute( IATTR_PLANES_ALPHA );
    if( alphaPlanes > 0 || alphaPlanes == eq::AUTO )
    {
        const int alphaBits = ( alphaPlanes > 0 ? alphaPlanes : 8 );
        format.setAlphaBufferSize( alphaBits );
    }

    const int depthPlanes = getAttribute( IATTR_PLANES_DEPTH );
    if( depthPlanes > 0  || depthPlanes == eq::AUTO )
    {
        const int depthBits = ( depthPlanes > 0 ? depthPlanes : 8 );
        format.setDepthBufferSize( depthBits );
    }
    else
        format.setDepthBufferSize( 0 );

    const int stencilPlanes = getAttribute( IATTR_PLANES_STENCIL );
    if( stencilPlanes > 0 || stencilPlanes == eq::AUTO )
    {
        const int stencilBits = ( stencilPlanes > 0 ? stencilPlanes : 8 );
        format.setStencilBufferSize( stencilBits );
    }
    else
        format.setStencilBufferSize( 0 );

    const int samplesPlanes  = getAttribute( IATTR_PLANES_SAMPLES );
    if( samplesPlanes >= 0 )
    {
        format.setSamples( samplesPlanes );
    }

    if( getAttribute( IATTR_HINT_STEREO ) == eq::ON ||
        ( getAttribute( IATTR_HINT_STEREO ) == eq::AUTO &&
          getAttribute( IATTR_HINT_DRAWABLE ) == eq::WINDOW ) )
    {
        format.setStereo( true );
    }

    if( getAttribute( IATTR_HINT_DOUBLEBUFFER ) == eq::ON ||
        ( getAttribute( IATTR_HINT_DOUBLEBUFFER ) == eq::AUTO &&
          getAttribute( IATTR_HINT_DRAWABLE ) == eq::WINDOW ))
    {
        format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
    }
    else
        format.setSwapBehavior( QSurfaceFormat::SingleBuffer );

    return format;
}

}

class Window
{
public:
    Window( WindowIF& parent_ )
        : _eventHandler( parent_ )
    {}

    virtual ~Window() {}
    virtual QOpenGLContext* getContext() const = 0;
    virtual void makeCurrent( const bool cache ) = 0;
    virtual void doneCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual bool configInit( eq::qt::Window& window ) = 0;

    virtual QObject *getEventProcessor()
    {
        return &_eventHandler;
    }

protected:
    EventHandler _eventHandler;
};

class QWindowWrapper : public Window, public QWindow
{
public:
    QWindowWrapper( WindowIF& parent_,
                    const WindowSettings& settings,
                    QOpenGLContext* shareContext = 0 )
        : detail::Window( parent_ )
        , _context( new QOpenGLContext )
        , _isCurrent( false )
        , _exposed( false )
    {
        const QSurfaceFormat& format_ = _createFormat( settings );
        setFormat( format_ );
        const PixelViewport& pvp = settings.getPixelViewport();
        setGeometry(pvp.x, pvp.y, pvp.w, pvp.h);
        setSurfaceType( QSurface::OpenGLSurface );

        _context->setFormat( format( ));
        _context->setShareContext( shareContext );
    }

    QOpenGLContext* getContext() const final
    {
        return _context.data();
    }

    void makeCurrent( const bool cache ) final
    {
        if( !_isCurrent || !cache )
        {
            _isCurrent = _context->makeCurrent( this );
            if( !_isCurrent )
                LBWARN << "QOpenGLContext::makeCurrent failed: "
                       << _context.data() << std::endl;
        }
    }

    void doneCurrent() final
    {
        _context->doneCurrent();
        _isCurrent = false;
    }

    void swapBuffers() final
    {
        // This is done to avoid a warning from Qt when the window has been
        // unexposed (e.g. prematurely closed).
        if( !isExposed( ))
            return;
        _context->swapBuffers( this );
    }

    bool configInit( eq::qt::Window& window ) final
    {
        show();

        // The application thread must be running the event loop somewhere
        // else. Otherwise this code will wait forever.
        _exposed.waitEQ( true );

        if( !_context->create( ))
            return false;

        // Adjusting the actual window viewport in case the window manager
        // disobeyed the requested geometry.
        _adjustEqWindowSize( window );

        return true;
    }

    QObject *getEventProcessor() final
    {
        return this;
    }

protected:
    void exposeEvent( QExposeEvent* qevent ) final
    {
        _exposed = !qevent->region().isEmpty();
        _eventHandler.exposeEvent();
    }

    void hideEvent( QHideEvent* ) final
    {
        _exposed = false;
        _eventHandler.hideEvent();
    }

    bool event( QEvent *qevent )
    {
        if (qevent->type() == QEvent::Close)
            _eventHandler.closeEvent();

        return QWindow::event( qevent );
    }

    void resizeEvent( QResizeEvent* qevent ) override
    {
        _eventHandler.resizeEvent( qevent );
    }

    void mousePressEvent( QMouseEvent* qevent ) override
    {
        _eventHandler.mousePressEvent( qevent );
    }

    void mouseReleaseEvent( QMouseEvent* qevent ) override
    {
        _eventHandler.mouseReleaseEvent( qevent );
    }

    void mouseMoveEvent( QMouseEvent* qevent ) override
    {
        _eventHandler.mouseMoveEvent( qevent );
    }

#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent* qevent ) override
    {
        _eventHandler.wheelEvent( qevent );
    }
#endif
    void keyPressEvent( QKeyEvent* qevent ) override
    {
        _eventHandler.keyPressEvent( qevent );
    }

    void keyReleaseEvent( QKeyEvent* qevent ) override
    {
        _eventHandler.keyReleaseEvent( qevent );
    }

private:
    QScopedPointer<QOpenGLContext> _context;
    bool _isCurrent;

    lunchbox::Monitorb _exposed;

    void _adjustEqWindowSize( eq::qt::Window& window )
    {
        PixelViewport pvp;
        pvp.x = x();
        pvp.y = y();
        pvp.w = width();
        pvp.h = height();
        window.setPixelViewport( pvp );
    }
};

class QOffscreenSurfaceWrapper : public Window, public QOffscreenSurface
{
public:
    QOffscreenSurfaceWrapper( WindowIF& parent_,
                              const WindowSettings& settings,
                              QOpenGLContext* shareContext = 0 )
        : detail::Window( parent_ )
        , _context( new QOpenGLContext )
        , _isCurrent( false )
    {
        const QSurfaceFormat& format_ = _createFormat( settings );
        setFormat( format_ );

        _context->setFormat( format( ));
        _context->setShareContext( shareContext );
    }

    QOpenGLContext* getContext() const final
    {
        return _context.data();
    }

    void makeCurrent( const bool cache ) final
    {
        if( !_isCurrent || !cache )
        {
            _isCurrent = _context->makeCurrent( this );
            if( !_isCurrent )
                LBWARN << "QOpenGLContext::makeCurrent failed: "
                       << _context.data() << std::endl;
        }
    }

    void doneCurrent() final
    {
        _context->doneCurrent();
        _isCurrent = false;
    }

    void swapBuffers() final
    {
        if( !isValid())

            return;
        _context->swapBuffers( this );
    }

    bool configInit( eq::qt::Window& ) final
    {
        // According to Qt docs, this doesn't work on platforms that require
        // allocation of the surface from the main thread (but I guess this is
        // mainly the case of mobile OSes)
        create();

        if( !isValid( ))
            return false;

        if( !_context->create( ))
            return false;

        return true;
    }

private:
    QScopedPointer<QOpenGLContext> _context;
    bool _isCurrent;
};

namespace
{
Window* _createWindow( WindowIF& parent,
                       const WindowSettings& settings,
                       QOpenGLContext* sharedContext )
{
    const int32_t drawable =
        settings.getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE );
    if( drawable == eq::PBUFFER || drawable == eq::FBO )
        return new QOffscreenSurfaceWrapper( parent, settings, sharedContext );
    return new QWindowWrapper( parent, settings, sharedContext );
}
}

}

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                QOpenGLContext* sharedContext )
    : WindowIF( parent, settings )
    , _impl( detail::_createWindow( *this, settings, sharedContext ))
{
}

Window::~Window()
{
    delete _impl;
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
}

QOpenGLContext* Window::getContext() const
{
    return _impl->getContext();
}

void Window::makeCurrent( const bool cache ) const
{
    _impl->makeCurrent( cache );
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

bool Window::processEvent( const WindowEvent& event )
{
    // Resizing the FBO if needed
    if( getFrameBufferObject() &&
        event.eq::Event::type == eq::Event::WINDOW_RESIZE )
    {
        getFrameBufferObject()->resize( event.resize.w, event.resize.h );
    }
    return SystemWindow::processEvent( event );
}

QObject* Window::getEventProcessor()
{
    return _impl->getEventProcessor();
}

}
}
