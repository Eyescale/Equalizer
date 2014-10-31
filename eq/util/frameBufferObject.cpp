
/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/client/gl.h>
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
    for( size_t i = 0; i < _colors.size(); ++i )
    {
        delete _colors[i];
        _colors[i] = 0;
    }
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
                               const int32_t stencilSize )
{
    LB_TS_THREAD( _thread );

    if( _fboID )
        return Error( ERROR_FRAMEBUFFER_INITIALIZED );

    // generate and bind the framebuffer
    glGenFramebuffersEXT( 1, &_fboID );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );

    // create and bind textures
    for( unsigned i = 0; i < _colors.size(); ++i )
    {
        _colors[i]->init( colorFormat, width, height );
        _colors[i]->bindToFBO( GL_COLOR_ATTACHMENT0 + i, width, height );
    }
    if( stencilSize > 0 && GLEW_EXT_packed_depth_stencil )
    {
        _depth.init( GL_DEPTH24_STENCIL8, width, height );
        _depth.bindToFBO( GL_DEPTH_STENCIL_ATTACHMENT, width, height );
    }
    else if( depthSize > 0 )
    {
        _depth.init( GL_DEPTH_COMPONENT, width, height );
        _depth.bindToFBO( GL_DEPTH_ATTACHMENT, width, height );
    }

    const Error error = _checkStatus();
    if( error )
        exit();
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

void FrameBufferObject::bind()
{
    LB_TS_THREAD( _thread );
    LBASSERT( _fboID );
    EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID ));
}

void FrameBufferObject::unbind()
{
    EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ));
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
