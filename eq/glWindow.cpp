
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Makhinya Maxim
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
        : glewContext( nullptr )
        , fbo( nullptr )
        , fboMultiSample( nullptr )
    {}

    ~GLWindow()
    {
        delete glewContext;
    }

    /** Extended OpenGL function entries when window has a context. */
    GLEWContext* glewContext;

    /** Frame buffer object for FBO drawables. */
    util::FrameBufferObject* fbo;

    util::FrameBufferObject* fboMultiSample;
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

void GLWindow::doneCurrent() const
{
    if( !_current )
        return;

    if( _impl->fbo )
        _impl->fbo->unbind();
    _current = 0;
}

bool GLWindow::isCurrent() const
{
    return _current == this;
}

void GLWindow::initGLEW()
{
    if( _impl->glewContext )
        return;

    _impl->glewContext = new GLEWContext;

#ifdef __linux__
    // http://sourceforge.net/p/glew/patches/40/
    glewExperimental = true;
#endif

    const GLenum result = glewInit();
    glGetError(); // eat GL errors from buggy glew implementation
    if( result == GLEW_OK )
        return;

    LBWARN << "GLEW initialization failed: " << std::endl;
    delete _impl->glewContext;
    _impl->glewContext = nullptr;
}

void GLWindow::exitGLEW()
{
    delete _impl->glewContext;
    _impl->glewContext = nullptr;
}

const util::FrameBufferObject* GLWindow::getFrameBufferObject() const
{
    return _impl->fbo;
}

util::FrameBufferObject* GLWindow::getFrameBufferObject()
{
    return _impl->fbo;
}

const GLEWContext* GLWindow::glewGetContext() const
{
    return _impl->glewContext;
}

GLEWContext* GLWindow::glewGetContext()
{
    return _impl->glewContext;
}

bool GLWindow::configInitFBO()
{
    if( !_impl->glewContext || !GLEW_EXT_framebuffer_object )
    {
        sendError( ERROR_FBO_UNSUPPORTED );
        return false;
    }

    if( !_createFBO( _impl->fbo, 0 ))
        return false;

    const int samplesSize = getIAttribute(WindowSettings::IATTR_PLANES_SAMPLES);
    if( samplesSize <= 0 )
        return true;

    return _createFBO( _impl->fboMultiSample, samplesSize );
}

void GLWindow::configExitFBO()
{
    _destroyFBO( _impl->fboMultiSample );
    _destroyFBO( _impl->fbo );
}

bool GLWindow::_createFBO( util::FrameBufferObject*& fbo, const int samplesSize)
{
    const PixelViewport& pvp = getPixelViewport();
    const GLuint colorFormat = getColorFormat();

    int depthSize = getIAttribute( WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize == AUTO )
        depthSize = 24;

    int stencilSize = getIAttribute( WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize == AUTO )
        stencilSize = 1;

    fbo = new util::FrameBufferObject( _impl->glewContext,
                                       samplesSize ? GL_TEXTURE_2D_MULTISAMPLE
                                                  : GL_TEXTURE_RECTANGLE_ARB );
    Error error = fbo->init( pvp.w, pvp.h, colorFormat, depthSize,
                             stencilSize, samplesSize );
    if( !error )
        return true;

    if( getIAttribute( WindowSettings::IATTR_PLANES_STENCIL ) == AUTO )
        error = fbo->init( pvp.w, pvp.h, colorFormat, depthSize, 0,
                           samplesSize );

    if( !error )
        return true;

    sendError( error.getCode( ));
    delete fbo;
    fbo = 0;
    return false;
}

void GLWindow::_destroyFBO( util::FrameBufferObject*& fbo )
{
    if( !fbo )
        return;

    fbo->exit();
    delete fbo;
    fbo = 0;
}

void GLWindow::bindFrameBuffer() const
{
   if( !_impl->glewContext )
       return;

   if( _impl->fbo )
       _impl->fbo->bind();
   else if( GLEW_EXT_framebuffer_object )
       glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void GLWindow::bindDrawFrameBuffer() const
{
    if( !_impl->glewContext )
        return;

    if( _impl->fboMultiSample )
        _impl->fboMultiSample->bind();
    else
        bindFrameBuffer();
}

void GLWindow::updateFrameBuffer() const
{
    if( !_impl->glewContext || !_impl->fboMultiSample )
        return;

    _impl->fboMultiSample->bind( GL_READ_FRAMEBUFFER_EXT );
    _impl->fbo->bind( GL_DRAW_FRAMEBUFFER_EXT );
    const PixelViewport& pvp = getPixelViewport();
    EQ_GL_CALL( glBlitFramebuffer( 0, 0, pvp.w, pvp.h,
                                   0, 0, pvp.w, pvp.h,
                                   GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                                   GL_STENCIL_BUFFER_BIT, GL_NEAREST ));
}

void GLWindow::flush()
{
    glFlush();
}

void GLWindow::finish()
{
    glFinish();
}

#define TEST_GLEW_VERSION( MAJOR, MINOR ) \
    if( GLEW_VERSION_ ## MAJOR ## _ ## MINOR )  \
        dc.glewGLVersion = MAJOR ## . ## MINOR ## f;    \

void GLWindow::queryDrawableConfig( DrawableConfig& dc )
{
    dc = DrawableConfig();

    // GL version
    const char* glVersion = (const char*)glGetString( GL_VERSION );
    if( !glVersion ) // most likely no context
    {
        LBWARN << "glGetString(GL_VERSION) returned 0, assuming GL version 1.1"
               << std::endl;
        dc.glVersion = 1.1f;
    }
    else
        dc.glVersion = static_cast<float>( std::atof( glVersion ));

    if( dc.glVersion >= 3.2f )
    {
        GLint mask;
        EQ_GL_CALL( glGetIntegerv( GL_CONTEXT_PROFILE_MASK, &mask ));
        dc.coreProfile = mask & GL_CONTEXT_CORE_PROFILE_BIT;
    }

    TEST_GLEW_VERSION( 1, 1 );
    TEST_GLEW_VERSION( 1, 2 );
    TEST_GLEW_VERSION( 1, 3 );
    TEST_GLEW_VERSION( 1, 4 );
    TEST_GLEW_VERSION( 1, 5 );
    TEST_GLEW_VERSION( 2, 0 );
    TEST_GLEW_VERSION( 2, 1 );
    TEST_GLEW_VERSION( 3, 0 );
    TEST_GLEW_VERSION( 3, 1 );
    TEST_GLEW_VERSION( 3, 2 );
    TEST_GLEW_VERSION( 3, 3 );
    TEST_GLEW_VERSION( 4, 0 );
    TEST_GLEW_VERSION( 4, 1 );
    TEST_GLEW_VERSION( 4, 2 );
    TEST_GLEW_VERSION( 4, 3 );
#ifdef GLEW_VERSION_4_5
    TEST_GLEW_VERSION( 4, 4 );
    TEST_GLEW_VERSION( 4, 5 );
#endif

    // Framebuffer capabilities
    GLboolean result;
    EQ_GL_CALL( glGetBooleanv( GL_STEREO, &result ));
    dc.stereo = result;

    EQ_GL_CALL( glGetBooleanv( GL_DOUBLEBUFFER, &result ));
    dc.doublebuffered = result;

    if( dc.coreProfile )
    {
        if( getFrameBufferObject( ))
        {
            glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &dc.stencilBits );
            // eat GL error if no stencil attachment; should return '0' bits
            // according to spec, but gives GL_INVALID_OPERATION
            glGetError();
            EQ_GL_CALL( glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
                                                               &dc.colorBits ));
            EQ_GL_CALL( glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
                                                               &dc.alphaBits ));
        }
        else
        {
            EQ_GL_CALL( glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_FRONT_LEFT, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                                               &dc.stencilBits ));
            EQ_GL_CALL( glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_FRONT_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
                                                               &dc.colorBits ));
            EQ_GL_CALL( glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
                GL_FRONT_LEFT, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
                                                               &dc.alphaBits ));
        }
    }
    else
    {
        EQ_GL_CALL( glGetIntegerv( GL_STENCIL_BITS, &dc.stencilBits ));
        EQ_GL_CALL( glGetIntegerv( GL_RED_BITS, &dc.colorBits ));
        EQ_GL_CALL( glGetIntegerv( GL_ALPHA_BITS, &dc.alphaBits ));
        EQ_GL_CALL( glGetIntegerv( GL_ACCUM_RED_BITS, &dc.accumBits ));
    }

    dc.accumBits *= 4;
    LBDEBUG << "Window drawable config: " << dc << std::endl;
}

}
