
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

#include "../window.h" // be first to avoid max/min name clashes on Win32...

#include "../windowSystem.h"

#include "glWidget.h"
#include "messagePump.h"
#include "window.h"
#include "../config.h"
#include "../pipe.h"

#include <co/objectICommand.h>
#include <eq/fabric/commands.h>
#include <QApplication>
#include <QThread>

namespace eq
{
namespace qt
{

class WindowSystem;
typedef co::CommandFunc< WindowSystem > WidgetFunc;

static class WindowSystem : WindowSystemIF, public co::Dispatcher
{
private:
    eq::WindowSystem _getSystemWindowSystem() const
    {
#ifdef AGL
        return eq::WindowSystem( "AGL" );
#elif GLX
        return eq::WindowSystem( "GLX" );
#elif WGL
        return eq::WindowSystem( "WGL" );
#endif
    }

#define getAttribute( attr ) settings.getIAttribute( WindowSettings::attr )

    bool _useSystemWindowSystem( const WindowSettings& settings,
                                 const eq::Window* sharedWindow )
    {
        return getAttribute( IATTR_HINT_DRAWABLE ) != eq::WINDOW ||
               (sharedWindow && sharedWindow->getSettings().getIAttribute(
                          WindowSettings::IATTR_HINT_DRAWABLE ) != eq::WINDOW );
    }

    std::string getName() const final { return QApplication::instance() ? "Qt"
                                                                        : ""; }

    eq::SystemWindow* createWindow( eq::Window* window,
                                    const WindowSettings& settings ) final
    {
        if( _useSystemWindowSystem( settings, window->getSharedContextWindow()))
            return _getSystemWindowSystem().createWindow( window, settings );

        LBINFO << "Using qt::Window" << std::endl;

        // need to create QGLWidget from main thread
        window->registerCommand( fabric::CMD_WINDOW_CREATE_QGL_WIDGET,
                        WidgetFunc( this, &WindowSystem::_cmdCreateQGLWidget ),
                                    window->getConfig()->getMainThreadQueue( ));

        co::LocalNodePtr localNode = window->getConfig()->getLocalNode();
        lunchbox::Request< void* > request =
                localNode->registerRequest< void* >();
        window->send( localNode, fabric::CMD_WINDOW_CREATE_QGL_WIDGET )
                << window << &settings << QThread::currentThread() << request;

        GLWidget* widget = reinterpret_cast< GLWidget* >( request.wait( ));
        return new Window( *window, settings, widget );
    }

    void destroyWindow( eq::Window* window ) final
    {
        const WindowSettings& settings = window->getSettings();
        if( _useSystemWindowSystem( settings, window->getSharedContextWindow()))
        {
            _getSystemWindowSystem().destroyWindow( window );
            return;
        }

        co::LocalNodePtr localNode = window->getConfig()->getLocalNode();

        // need to destroy QGLWidget from main thread
        window->registerCommand( fabric::CMD_WINDOW_DESTROY_QGL_WIDGET,
                         WidgetFunc( this, &WindowSystem::_cmdDestroyGLWidget ),
                                    window->getConfig()->getMainThreadQueue( ));

        Window* sysWindow = static_cast< Window* >( window->getSystemWindow( ));
        window->send( localNode, fabric::CMD_WINDOW_DESTROY_QGL_WIDGET )
                << sysWindow->getGLWidget();
    }

    eq::SystemPipe* createPipe( eq::Pipe* pipe ) final
    {
        return _getSystemWindowSystem().createPipe( pipe );
    }

    eq::MessagePump* createMessagePump() final
    {
        return new MessagePump;
    }

    bool setupFont( util::ObjectManager& gl LB_UNUSED,
                    const void* key LB_UNUSED,
                    const std::string& name LB_UNUSED,
                    const uint32_t size LB_UNUSED ) const final
    {
#ifdef AGL
        return true;
#else
        return _getSystemWindowSystem().setupFont( gl, key, name, size );
#endif
    }

    QGLFormat _createQGLFormat( const WindowSettings& settings )
    {
        // defaults: http://qt-project.org/doc/qt-4.8/qglformat.html#QGLFormat
        QGLFormat format;

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
            format.setDepth( false );

        const int stencilPlanes = getAttribute( IATTR_PLANES_STENCIL );
        if( stencilPlanes > 0 || stencilPlanes == eq::AUTO )
        {
            format.setStencil( true );
            const int stencilBits = ( stencilPlanes > 0 ? stencilPlanes : 8 );
            format.setStencilBufferSize( stencilBits );
        }
        else
            format.setStencil( false );

        // Qt only allows only the same bit depth for each channel
        // http://qt-project.org/doc/qt-4.8/qglformat.html#setAccumBufferSize
        const int accumPlanes  = getAttribute( IATTR_PLANES_ACCUM );
        const int accumAlphaPlanes = getAttribute( IATTR_PLANES_ACCUM_ALPHA );
        const int accumBits = std::max( accumPlanes, accumAlphaPlanes );
        if( accumBits >= 0 )
        {
            format.setAccum( true );
            format.setAccumBufferSize( accumBits );
        }

        const int samplesPlanes  = getAttribute( IATTR_PLANES_SAMPLES );
        if( samplesPlanes >= 0 )
        {
            format.setSampleBuffers( true );
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
            format.setDoubleBuffer( true );
        }
        else
            format.setDoubleBuffer( false );

        return format;
    }

    bool _cmdCreateQGLWidget( co::ICommand& command )
    {
        co::ObjectICommand cmd( command );

        eq::Window* window =
                reinterpret_cast< eq::Window* >( cmd.get< void* >( ));
        WindowSettings& settings =
                *reinterpret_cast< WindowSettings* >( cmd.get< void* >( ));

        const GLWidget* shareGLWidget = 0;
        const SystemWindow* shareContextWindow =
                            window->getSharedContextWindow()->getSystemWindow();
        if( shareContextWindow )
        {
            const Window* qtWindow =
                             static_cast< const Window* >( shareContextWindow );
            shareGLWidget = qtWindow->getGLWidget();
        }

        GLWidget* glWidget = new GLWidget( _createQGLFormat( settings ),
                                           shareGLWidget );
        PixelViewport pvp;
        if( getAttribute( IATTR_HINT_FULLSCREEN ) == eq::ON )
        {
            pvp = window->getPipe()->getPixelViewport();
            glWidget->showFullScreen();
        }
        else
        {
            pvp = settings.getPixelViewport();
            glWidget->show();
        }
        glWidget->setWindowTitle( QString::fromStdString( settings.getName()));
        glWidget->setGeometry( pvp.x, pvp.y, pvp.w, pvp.h );
        glWidget->doneCurrent();

#if QT_VERSION >= 0x050000
        QThread* renderThread =
                reinterpret_cast< QThread* >( cmd.get< void* >( ));
        glWidget->context()->moveToThread( renderThread );
#else
        cmd.get< void* >();
#endif

        window->getLocalNode()->serveRequest( cmd.get< uint32_t >(), glWidget );
        return true;
    }

    bool _cmdDestroyGLWidget( co::ICommand& command )
    {
        co::ObjectICommand cmd( command );
        GLWidget* glWidget = reinterpret_cast< GLWidget* >( cmd.get< void* >());
        delete glWidget;
        return true;
    }

} _qtFactory;

}
}
