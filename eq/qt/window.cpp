
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

#include "windowEvent.h"
#include "windowSystem.h"
#include "eventHandler.h"

#include <eq/util/frameBufferObject.h>
#include <lunchbox/monitor.h>

#include <QApplication>
#include <QEventLoop>
#include <QExposeEvent>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QPointer>
#include <QWindow>

namespace eq
{
namespace qt
{
namespace
{
#define getAttribute( attr ) settings.getIAttribute( WindowSettings::attr )

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

    if( getAttribute( IATTR_HINT_STEREO ) == eq::ON )
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

namespace detail
{
class Window
{
public:
    Window() : _eventHandler( 0 ) {}

    virtual ~Window() {}
    virtual QOpenGLContext* getContext() const = 0;
    virtual void makeCurrent() = 0;
    virtual void doneCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual bool configInit( eq::qt::Window& window )
    {
        _eventHandler = new EventHandler( window );
        return true;
    }
    virtual bool configExit()
    {
        delete _eventHandler;
        _eventHandler = 0;
        return true;
    }

    virtual QObject *getEventProcessor() { return _eventHandler; }

protected:
    EventHandler* _eventHandler;
};

class QWindowWrapper : public Window, public QWindow
{
public:
    QWindowWrapper( const WindowSettings& settings,
                    QOpenGLContext* shareContext )
        : _context( new QOpenGLContext )
        , _exposed( false )
    {
        const QSurfaceFormat& format_ = _createFormat( settings );
        setFormat( format_ );
        const PixelViewport& pvp = settings.getPixelViewport();
        setGeometry( pvp.x, pvp.y, pvp.w, pvp.h );
        setSurfaceType( QSurface::OpenGLSurface );

        _context->setFormat( format( ));
        _context->setShareContext( shareContext );
        show();
    }

    QOpenGLContext* getContext() const final
    {
        return _context.data();
    }

    void makeCurrent() final
    {
        if( !_context->makeCurrent( this ))
            LBWARN << "QOpenGLContext::makeCurrent failed: "
                   << _context.data() << std::endl;
    }

    void doneCurrent() final
    {
        _context->doneCurrent();
        _isCurrent = false;
    }

    void swapBuffers() final
    {
        if( isExposed( ))
            _context->swapBuffers( this );
    }

    bool configInit( eq::qt::Window& window ) final
    {
        if( !Window::configInit( window ))
            return false;

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

    QObject *getEventProcessor() final { return this; }

protected:
    void exposeEvent( QExposeEvent* qevent ) final
    {
        _exposed = !qevent->region().isEmpty();
        _eventHandler->exposeEvent();
    }

    void hideEvent( QHideEvent* ) final
    {
        _exposed = false;
        _eventHandler->hideEvent();
    }

    bool event( QEvent *qevent )
    {
        if (qevent->type() == QEvent::Close)
            _eventHandler->closeEvent();

        return QWindow::event( qevent );
    }

    void resizeEvent( QResizeEvent* qevent ) override
    {
        _eventHandler->resizeEvent( qevent );
    }

    void mousePressEvent( QMouseEvent* qevent ) override
    {
        _eventHandler->mousePressEvent( qevent );
    }

    void mouseReleaseEvent( QMouseEvent* qevent ) override
    {
        _eventHandler->mouseReleaseEvent( qevent );
    }

    void mouseMoveEvent( QMouseEvent* qevent ) override
    {
        _eventHandler->mouseMoveEvent( qevent );
    }

#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent* qevent ) override
    {
        _eventHandler->wheelEvent( qevent );
    }
#endif
    void keyPressEvent( QKeyEvent* qevent ) override
    {
        _eventHandler->keyPressEvent( qevent );
    }

    void keyReleaseEvent( QKeyEvent* qevent ) override
    {
        _eventHandler->keyReleaseEvent( qevent );
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
    QOffscreenSurfaceWrapper( const WindowSettings& settings,
                              QOpenGLContext* shareContext )
        : _context( new QOpenGLContext )
        , _isCurrent( false )
    {
        const QSurfaceFormat& format_ = _createFormat( settings );
        setFormat( format_ );

        _context->setFormat( format( ));
        _context->setShareContext( shareContext );
        create();
    }

    QOpenGLContext* getContext() const final { return _context.data(); }

    void makeCurrent() final
    {
        if( _context->makeCurrent( this ))
            LBWARN << "QOpenGLContext::makeCurrent failed: "
                   << _context.data() << std::endl;
    }

    void doneCurrent() final
    {
        _context->doneCurrent();
        _isCurrent = false;
    }

    void swapBuffers() final
    {
        if( isValid( ))
            _context->swapBuffers( this );
    }

    bool configInit( eq::qt::Window& window ) final
    {
        return Window::configInit( window ) && isValid() && _context->create();
    }

private:
    QScopedPointer< QOpenGLContext > _context;
    bool _isCurrent;
};
}

detail::Window* Window::createImpl( const WindowSettings& settings,
                                    QOpenGLContext* sharedContext,
                                    QThread* thread LB_UNUSED )
{
    const int32_t drawable = getAttribute( IATTR_HINT_DRAWABLE );
    detail::Window* window = 0;
    if( drawable == eq::WINDOW )
        window = new detail::QWindowWrapper( settings, sharedContext );
    else
        window = new detail::QOffscreenSurfaceWrapper( settings, sharedContext);

#if QT_VERSION >= 0x050000
    window->getContext()->moveToThread( thread );
#endif
    LBASSERT( window );
    return window;
}

void Window::onDestroyImpl( detail::Window* window )
{
    delete window;
}

Window::Window( NotifierInterface& parent_, const WindowSettings& settings,
                detail::Window* impl )
    : WindowIF( parent_, settings )
    , _impl( impl )
{
    LBASSERT( impl );
    QCoreApplication* app = QApplication::instance();
    app->connect( this, SIGNAL( destroyImpl( detail::Window* )),
                  this, SLOT( onDestroyImpl( detail::Window* )));
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

void Window::makeCurrent( const bool cache ) const
{
    if( cache && isCurrent( ))
        return;

    _impl->makeCurrent(); // Make real GL context current first
    WindowIF::makeCurrent(); // Validate FBO binding and caching state
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
