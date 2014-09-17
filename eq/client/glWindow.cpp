
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *               2012-2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2009, Makhinya Maxim
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

#include "glWindow.h"

#include "error.h"
#include "gl.h"
#include "global.h"
#include "pipe.h"

#include <eq/fabric/drawableConfig.h>
#include <eq/util/frameBufferObject.h>
#include <lunchbox/os.h>
#include <lunchbox/perThread.h>

template
void lunchbox::perThreadNoDelete< const eq::GLWindow >( const eq::GLWindow* );

namespace eq
{
namespace
{
lunchbox::PerThread< const GLWindow, lunchbox::perThreadNoDelete > _current;
}
namespace detail
{
class GLWindow
{
public:
    GLWindow()
        : glewInitialized( false )
        , fbo( 0 )
    {
        lunchbox::setZero( &glewContext, sizeof( GLEWContext ));
    }

    ~GLWindow()
    {
        glewInitialized = false;
#ifndef NDEBUG
        lunchbox::setZero( &glewContext, sizeof( GLEWContext ));
#endif
    }

    bool glewInitialized ;

    /** Extended OpenGL function entries when window has a context. */
    GLEWContext glewContext;

    /** Frame buffer object for FBO drawables. */
    util::FrameBufferObject* fbo;
};
}

GLWindow::GLWindow( NotifierInterface& parent, const WindowSettings& settings )
    : SystemWindow( parent, settings )
    , _impl( new detail::GLWindow )
{
}

GLWindow::~GLWindow()
{
    if( _current == this )
        _current = 0;
    delete _impl;
}

void GLWindow::makeCurrent( const bool useCache ) const
{
    if( useCache && _current == this )
        return;

    bindFrameBuffer();
    _current = this;
}

bool GLWindow::isCurrent() const
{
    return _current == this;
}

void GLWindow::initGLEW()
{
    if( _impl->glewInitialized )
        return;

    const GLenum result = glewInit();
    if( result != GLEW_OK )
        LBWARN << "GLEW initialization failed: " << std::endl;
    else
        _impl->glewInitialized = true;
}

void GLWindow::exitGLEW()
{
    _impl->glewInitialized = false;
}

const util::FrameBufferObject* GLWindow::getFrameBufferObject() const
{
    return _impl->fbo;
}

const GLEWContext* GLWindow::glewGetContext() const
{
    return &_impl->glewContext;
}

GLEWContext* GLWindow::glewGetContext()
{
    return &_impl->glewContext;
}

bool GLWindow::configInitFBO()
{
    if( !_impl->glewInitialized ||
        !GLEW_ARB_texture_non_power_of_two || !GLEW_EXT_framebuffer_object )
    {
        sendError( ERROR_FBO_UNSUPPORTED );
        return false;
    }

    // needs glew initialized (see above)
    _impl->fbo = new util::FrameBufferObject( &_impl->glewContext );

    const PixelViewport& pvp = getPixelViewport();
    const GLuint colorFormat = getColorFormat();

    int depthSize = getIAttribute( WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize == AUTO )
         depthSize = 24;

    int stencilSize = getIAttribute( WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize == AUTO )
        stencilSize = 1;

    Error error = _impl->fbo->init( pvp.w, pvp.h, colorFormat, depthSize,
                                    stencilSize );
    if( !error )
        return true;

    if( getIAttribute( WindowSettings::IATTR_PLANES_STENCIL ) == AUTO )
        error = _impl->fbo->init( pvp.w, pvp.h, colorFormat, depthSize, 0 );

    if( !error )
        return true;

    sendError( error.getCode( ));
    delete _impl->fbo;
    _impl->fbo = 0;
    return false;
}

void GLWindow::configExitFBO()
{
    if( _impl->fbo )
        _impl->fbo->exit();

    delete _impl->fbo;
    _impl->fbo = 0;
}

void GLWindow::bindFrameBuffer() const
{
   if( !_impl->glewInitialized )
       return;

   if( _impl->fbo )
       _impl->fbo->bind();
   else if( GLEW_EXT_framebuffer_object )
       glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void GLWindow::flush()
{
    glFlush();
}

void GLWindow::finish()
{
    glFinish();
}

void GLWindow::queryDrawableConfig( DrawableConfig& drawableConfig )
{
    // GL version
    const char* glVersion = (const char*)glGetString( GL_VERSION );
    if( !glVersion ) // most likely no context - fail
    {
        LBWARN << "glGetString(GL_VERSION) returned 0, assuming GL version 1.1"
               << std::endl;
        drawableConfig.glVersion = 1.1f;
    }
    else
        drawableConfig.glVersion = static_cast<float>( atof( glVersion ));

    // Framebuffer capabilities
    GLboolean result;
    glGetBooleanv( GL_STEREO,       &result );
    drawableConfig.stereo = result;

    glGetBooleanv( GL_DOUBLEBUFFER, &result );
    drawableConfig.doublebuffered = result;

    GLint stencilBits;
    glGetIntegerv( GL_STENCIL_BITS, &stencilBits );
    drawableConfig.stencilBits = stencilBits;

    GLint colorBits;
    glGetIntegerv( GL_RED_BITS, &colorBits );
    drawableConfig.colorBits = colorBits;

    GLint alphaBits;
    glGetIntegerv( GL_ALPHA_BITS, &alphaBits );
    drawableConfig.alphaBits = alphaBits;

    GLint accumBits;
    glGetIntegerv( GL_ACCUM_RED_BITS, &accumBits );
    drawableConfig.accumBits = accumBits * 4;

    LBINFO << "Window drawable config: " << drawableConfig << std::endl;
}

}
