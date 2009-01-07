/* Copyright (c) 2008-2009, Cedric Stalder <cedric.stalder@gmail.com>
 All rights reserved. */

#include "frameBufferObject.h"
#include <eq/eq.h>

using namespace std; // for use endl

namespace eq
{
	
FrameBufferObject::FrameBufferObject( GLEWContext* glewContext )
    : _glewContext( glewContext )
    , _fboID(0){}
	
FrameBufferObject::~FrameBufferObject()
{
    
    if( _fboID )
    {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
        glDeleteFramebuffers( 1, &_fboID );
    }
    
    if( _textureID[COLOR_TEXTURE] )
    {      
        glDeleteTextures( 1, &_textureID[COLOR_TEXTURE] );   
    }
    
    if( _textureID[DEPTH_TEXTURE] )
    {      
        glDeleteTextures( 1, &_textureID[DEPTH_TEXTURE] );   
    }
    
    if( _textureID[STENCIL_TEXTURE] )
    {      
        glDeleteTextures( 1, &_textureID[STENCIL_TEXTURE] );   
    }
}
	
bool FrameBufferObject::init( const int width, const int height, 
                              const int depthSize, const int stencilSize )
{

    // generate the framebuffer and 3 texture object names
    glGenFramebuffersEXT( 1, &_fboID );
    glGenTextures( 1, &_textureID[COLOR_TEXTURE] );
    
    // Bind the frame Buffer
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );
	
    // creation texture for the color
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _textureID[COLOR_TEXTURE] );
    glTexImage2D ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, 
                   GL_RGBA, GL_UNSIGNED_BYTE, 0 );
    
    // specify texture as color attachment 
    glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_RECTANGLE_ARB, _textureID[COLOR_TEXTURE], 0 );

    if ( depthSize > 0 )
    {
        // creation texture for the depth 
        glGenTextures( 1, &_textureID[DEPTH_TEXTURE] );
        glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _textureID[DEPTH_TEXTURE] ); 
        glTexImage2D ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width, 
                       height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 
                       0 ); 
               
        // specify texture as depth attachment 
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                   GL_TEXTURE_RECTANGLE_ARB, 
                                  _textureID[DEPTH_TEXTURE], 0 );
    }

    if ( stencilSize > 0 )
    {
        glGenTextures( 1, &_textureID[STENCIL_TEXTURE] );
        // creation texture for the depth 
        glBindTexture( GL_TEXTURE_RECTANGLE_ARB, _textureID[STENCIL_TEXTURE] ); 
        glTexImage2D ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_STENCIL_INDEX, width, 
                       height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_SHORT, 
                       0 ); 
    
        // specify texture as depth attachment 
        glFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_STENCIL_INDEX, 
                                   GL_TEXTURE_RECTANGLE_ARB, 
                                  _textureID[STENCIL_TEXTURE], 0 );
    }

    return checkFBOStatus();
}

bool FrameBufferObject::checkFBOStatus()
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
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _fboID );
}


}	
	

	
