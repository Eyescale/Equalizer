
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *               2014, Stefan.Eilemann@epfl.ch
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
#include <eq/client/pipe.h>

#include "widgetFactory.h"

#include "glWidget.h"
#include <eq/client/window.h>
#include <QGLFormat>
#undef max

namespace eq
{
namespace qt
{

#define getAttribute( attr ) settings.getIAttribute( WindowSettings::attr )

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

GLWidget* WidgetFactory::onCreateWidget( eq::Window* window,
                                         const WindowSettings& settings,
                                         QThread* renderThread LB_UNUSED )
{
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
    const QString& title = QString::fromStdString( settings.getName( ) );
    glWidget->setWindowTitle( title.isEmpty() ? "Equalizer" : title );

    const bool isOnscreen = settings.getIAttribute(
                            WindowSettings::IATTR_HINT_DRAWABLE ) == eq::WINDOW;
    if( settings.getIAttribute(
            WindowSettings::IATTR_HINT_FULLSCREEN ) == eq::ON )
    {
        const PixelViewport& pvp = window->getPipe()->getPixelViewport();
        glWidget->setGeometry( pvp.x, pvp.y, pvp.w, pvp.h );
        if( isOnscreen )
            glWidget->showFullScreen();
    }
    else
    {
        const PixelViewport& pvp = isOnscreen ? settings.getPixelViewport()
                                              : window->getPipe()->getPixelViewport();
        glWidget->setGeometry( pvp.x, pvp.y, pvp.w, pvp.h );
        if( isOnscreen )
            glWidget->show();
    }
    glWidget->doneCurrent();

#if QT_VERSION >= 0x050000
    glWidget->context()->moveToThread( renderThread );
#endif
    return glWidget;
}

void WidgetFactory::onDestroyWidget( GLWidget* widget )
{
    delete widget;
}

}
}
