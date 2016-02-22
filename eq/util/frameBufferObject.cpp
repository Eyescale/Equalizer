
/* Copyright (c) 2008-2015, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "frameBufferObject.h"

#include <eq/gl.h>
#include <eq/fabric/pixelViewport.h>

#ifdef _WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

namespace eq
{
namespace util
{

FrameBufferObject::FrameBufferObject( const GLEWContext* glewContext,
                                      const GLenum textureTarget )
    : _fboID( 0 )
    , _depth( textureTarget, glewContext )
    , _glewContext( glewContext )
    , _valid( false )
{
    LBASSERT( GLEW_EXT_framebuffer_object );
    _colors.push_back( new Texture( textureTarget, glewContext ));
}

FrameBufferObject::~FrameBufferObject()
{
    this->exit();
    for( Texture* color : _colors )
        delete color;
}

bool FrameBufferObject::addColorTexture()
{
    if( _colors.size() >= 16 )
    {
        LBWARN << "Too many color textures, can't add another one" << std::endl;
        return false;
    }

    _colors.push_back( new Texture(_colors.front()->getTarget(), _glewContext));
    _valid = false;
    return true;
}

Error FrameBufferObject::init( const int32_t width, const int32_t height,
                               const GLuint colorFormat,
                               const int32_t depthSize,
                               const int32_t stencilSize,
                               const int32_t samplesSize )
{
    LB_TS_THREAD( _thread );

    // Check for frame dimensions
    GLint maxViewportDims[2];
    EQ_GL_CALL( glGetIntegerv( GL_MAX_VIEWPORT_DIMS, &maxViewportDims[0] ));
    if( width > maxViewportDims[0] || height > maxViewportDims[1] )
        return Error( ERROR_FRAMEBUFFER_INVALID_SIZE );

    // Check for MAX_SAMPLES
    GLint maxSamples;
    glGetIntegerv( GL_MAX_SAMPLES, &maxSamples );
    if( samplesSize < 0 || samplesSize > maxSamples )
        return Error( ERROR_FRAMEBUFFER_INVALID_SAMPLES );

    if( _fboID )
        return Error( ERROR_FRAMEBUFFER_INITIALIZED );

    // generate and bind the framebuffer
    EQ_GL_CALL( glGenFramebuffersEXT( 1, &_fboID ));
    EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID ));

    GLint mask;
    glGetIntegerv( GL_CONTEXT_PROFILE_MASK, &mask );
    const GLenum glError = glGetError(); // might get GL_INVALID_ENUM
    const bool coreContext =
        glError ? false : mask & GL_CONTEXT_CORE_PROFILE_BIT;

    // create and bind textures
    for( unsigned i = 0; i < _colors.size(); ++i )
    {
        _colors[i]->init( colorFormat, width, height );
        _colors[i]->bindToFBO( GL_COLOR_ATTACHMENT0 + i, width, height,
                               samplesSize );
        const Error error = _checkStatus();
        if( error )
        {
            LBDEBUG << "FrameBufferObject::init: " << error << " when binding "
                    << _colors.size() << " color texture(s) of format 0x"
                    << std::hex << colorFormat << std::dec << " size " << width
                    << "x" << height << " FBO " << _fboID << std::endl;
            exit();
            return error;
        }
    }
    if( stencilSize > 0 && ( GLEW_EXT_packed_depth_stencil || coreContext ))
    {
        _depth.init( GL_DEPTH24_STENCIL8, width, height );
        _depth.bindToFBO( GL_DEPTH_STENCIL_ATTACHMENT, width, height,
                          samplesSize );
        const Error error = _checkStatus();
        if( error )
        {
            LBDEBUG << "FrameBufferObject::init: " << error
                    << " when binding GL_DEPTH24_STENCIL8 texture" << std::endl;
            exit();
            return error;
        }
    }
    else if( depthSize > 0 )
    {
        _depth.init( GL_DEPTH_COMPONENT, width, height );
        _depth.bindToFBO( GL_DEPTH_ATTACHMENT, width, height, samplesSize );
        const Error error = _checkStatus();
        if( error )
        {
            LBDEBUG << "FrameBufferObject::init: " << error
                    << " when binding GL_DEPTH_COMPONENT texture" << std::endl;
            exit();
            return error;
        }
    }

    const Error error = _checkStatus();
    if( error )
    {
        LBDEBUG << "FrameBufferObject::init: " << error << std::endl;
        exit();
    }
    return error;
}

void FrameBufferObject::exit()
{
    LB_TS_THREAD( _thread );
    if( _fboID )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        glDeleteFramebuffersEXT( 1, &_fboID );
        _fboID = 0;
    }

    for( size_t i = 0; i < _colors.size(); ++i )
        _colors[i]->flush();
    _depth.flush();

    _valid = false;
}

Error FrameBufferObject::_checkStatus()
{
    _valid = false;

    const GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
    switch( status )
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        _valid = true;
        return Error( ERROR_NONE );

    case 0: // error?!
        EQ_GL_ERROR( "glCheckFramebufferStatusEXT" );
        return Error( ERROR_FRAMEBUFFER_STATUS );

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        return Error( ERROR_FRAMEBUFFER_UNSUPPORTED );

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT );

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_ATTACHMENT );

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_DIMENSIONS );

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_FORMATS );

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER );

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        return Error( ERROR_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );

    default:
        LBWARN << "Unhandled frame buffer status 0x" << std::hex
               << status << std::dec << std::endl;
        return Error( ERROR_FRAMEBUFFER_STATUS );
    }
}

void FrameBufferObject::bind( const uint32_t target )
{
    LB_TS_THREAD( _thread );
    LBASSERT( _fboID );
    EQ_GL_CALL( glBindFramebufferEXT( target, _fboID ));
}

void FrameBufferObject::unbind( const uint32_t target )
{
    EQ_GL_CALL( glBindFramebufferEXT( target, 0 ));
}

Error FrameBufferObject::resize( const int32_t width, const int32_t height )
{
    LB_TS_THREAD( _thread );
    LBASSERT( width > 0 && height > 0 );

    LBASSERT( !_colors.empty( ));
    Texture* color = _colors.front();

    if( color->getWidth() == width && color->getHeight() == height && _valid )
        return Error( ERROR_NONE );

    for( size_t i = 0; i < _colors.size(); ++i )
        _colors[i]->resize( width, height );

    if ( _depth.isValid( ))
        _depth.resize( width, height );

    return _checkStatus();
}

}
}
