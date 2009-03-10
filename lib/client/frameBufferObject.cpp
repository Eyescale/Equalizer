/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
                 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "frameBufferObject.h"
#include <eq/eq.h>

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

namespace eq
{
	
FrameBufferObject::FrameBufferObject( GLEWContext* glewContext )
    : _fboID(0)
    , _width(0)
    , _height(0)
    , _color( glewContext )
    , _depth( glewContext )
    , _stencil( glewContext )
    , _glewContext( glewContext )
{
    EQASSERT( GLEW_EXT_framebuffer_object );

    _color.setFormat( GL_RGBA );
    _depth.setFormat( GL_DEPTH_COMPONENT );
    _stencil.setFormat( GL_STENCIL_INDEX );
}

FrameBufferObject::~FrameBufferObject()
{
    exit();
}

void FrameBufferObject::exit()
{
    CHECK_THREAD( _thread );
    if( _fboID )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        glDeleteFramebuffersEXT( 1, &_fboID );
        _fboID = 0;
    }

    _color.flush();
    _depth.flush();
    _stencil.flush();
}

void FrameBufferObject::setColorFormat( const GLuint format )
{
    _color.setFormat( format );
}


bool FrameBufferObject::init( const int width, const int height, 
                              const int depthSize, const int stencilSize )
{
    CHECK_THREAD( _thread );

    if( _fboID )
    {
        _error = "FBO already initialized";
        EQWARN << _error << std::endl;
        return false;
    }

    // generate and bind the framebuffer
    glGenFramebuffersEXT( 1, &_fboID );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );

    // create and bind textures
    _color.bindToFBO( GL_COLOR_ATTACHMENT0, width, height );

    if ( depthSize > 0 )
        _depth.bindToFBO( GL_DEPTH_ATTACHMENT, width, height );

    if ( stencilSize > 0 )
        _stencil.bindToFBO( GL_STENCIL_ATTACHMENT, width, height );
    
    _width = width;
    _height = height;
    return _checkFBOStatus();
}

bool FrameBufferObject::_checkFBOStatus()
{
    switch( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ))
    {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            EQVERB << "FBO supported and complete" << std::endl;
            return true;
			
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            _error = "Unsupported framebuffer format";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            _error = "Framebuffer incomplete, missing attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            _error = "Framebuffer incomplete, incomplete attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            _error = "Framebuffer incomplete, attached images must have same dimensions";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            _error = "Framebuffer incomplete, attached images must have same format";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            _error = "Framebuffer incomplete, missing draw buffer";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            _error = "Framebuffer incomplete, missing read buffer";
            break;
        default:
            break;
    }

    EQERROR << _error << std::endl;
    return false;
}
	
void FrameBufferObject::bind()
{ 
    CHECK_THREAD( _thread );
    EQASSERT( _fboID );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );
}

void FrameBufferObject::unbind()
{
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ); 
}

bool FrameBufferObject::resize( const int width, const int height )
{
    CHECK_THREAD( _thread );
    EQASSERT( width > 0 && height > 0 );

    if (( _width == width ) && ( _height == height ))
       return true; 
     
    _color.resize( width, height );

    if ( _depth.isValid( ))
        _depth.resize( width, height );
    
    if ( _stencil.isValid( ))
        _stencil.resize( width, height );

    _width = width;
    _height = height;

    return _checkFBOStatus();
}
}	
	

	
