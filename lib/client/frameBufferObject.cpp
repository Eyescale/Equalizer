/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
                 2009, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

#include "frameBufferObject.h"
#include <eq/eq.h>

using namespace std;

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

namespace eq
{
	
FrameBufferObject::FrameBufferObject( GLEWContext* glewContext )
    : _fboID(0)
    , _width(0)
    , _height(0)
    , _glewContext( glewContext )
{
    EQASSERT( GLEW_EXT_framebuffer_object );
    _textures[ COLOR_TEXTURE ].setFormat( GL_RGBA );
    _textures[ DEPTH_TEXTURE ].setFormat( GL_DEPTH_COMPONENT );
    _textures[ STENCIL_TEXTURE ].setFormat( GL_STENCIL_INDEX );
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

    for( size_t i=i; i < ALL_TEXTURE; ++i )
        _textures[i].flush();
}
	
bool FrameBufferObject::init( const int width, const int height, 
                              const int depthSize, const int stencilSize )
{
    CHECK_THREAD( _thread );

    if( _fboID )
    {
        EQWARN << "FBO already initialized" << endl;
        return false;
    }

    // generate and bind the framebuffer
    glGenFramebuffersEXT( 1, &_fboID );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );

    // create and bind textures
    _textures[ COLOR_TEXTURE ].bindToFBO( GL_COLOR_ATTACHMENT0, width, height );

    if ( depthSize > 0 )
        _textures[ DEPTH_TEXTURE ].bindToFBO( GL_DEPTH_ATTACHMENT,
                                              width, height );

    if ( stencilSize > 0 )
        _textures[ STENCIL_TEXTURE ].bindToFBO( GL_STENCIL_ATTACHMENT,
                                                width, height );
    
    _width = width;
    _height = height;
    return checkFBOStatus();
}

bool FrameBufferObject::checkFBOStatus() const
{
	const GLenum status = ( GLenum ) glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
    switch( status ) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            EQVERB << "Frame Buffer Object supported and complete " << endl;
            return true;
			
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            EQERROR << "Unsupported framebuffer format " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            EQERROR << "Framebuffer incomplete, missing attachment " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            EQERROR << "Framebuffer incomplete, duplicate attachment " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            EQERROR << "Framebuffer incomplete, attached images must have same dimensions " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            EQERROR << "Framebuffer incomplete, attached images must have same format " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            EQERROR << "Framebuffer incomplete, missing draw buffer " << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            EQERROR << "Framebuffer incomplete, missing read buffer " << endl;
            break;
        default:
            break;
    }
    return false;
}
	
void FrameBufferObject::bind()
{ 
    CHECK_THREAD( _thread );
    EQASSERT( _fboID );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );
}

bool FrameBufferObject::resize( const int width, const int height )
{
    CHECK_THREAD( _thread );
    if (( _width == width ) && ( _height == height ))
       return true; 
     
    _textures[ COLOR_TEXTURE ].resize( width, height );

    if ( _textures[ DEPTH_TEXTURE ].isValid( ))
        _textures[ DEPTH_TEXTURE ].resize( width, height );
    
    if ( _textures[ STENCIL_TEXTURE ].isValid( ))
        _textures[ STENCIL_TEXTURE ].resize( width, height );

    _width = width;
    _height = height;

    return checkFBOStatus();
}
}	
	

	
