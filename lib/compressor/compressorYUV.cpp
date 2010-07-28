
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009, Maxim Makhinya
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "compressorYUV.h"
#include <eq/util/frameBufferObject.h>

#include <GL/glew.h>

#include "yuv420readback_glsl.h"
#include "yuv420unpack_glsl.h"

#define glewGetContext() glewContext

namespace eq
{
namespace plugin
{

/** Construct a new compressor Yuv */
CompressorYUV::CompressorYUV( const EqCompressorInfo* info )
        : Compressor( info )
        , _program( 0 )
        , _fbo( 0 )
        , _texture( 0 )
{ }

/** Destruct the compressor Yuv */
CompressorYUV::~CompressorYUV( )
{
    // replace original texture
    if( _fbo )
        delete _fbo;
    _fbo = 0;

    if ( _texture )
    {
        _texture->flush();
        delete _texture;
    }
    _texture = 0;
}

bool CompressorYUV::isCompatible( const GLEWContext* glewContext )
{
    return ( GL_ARB_texture_non_power_of_two && 
             GL_VERSION_2_0 && 
             GLEW_EXT_framebuffer_object );
}

void CompressorYUV::_init( const GLEWContext* glewContext,
                           const char* fShaderPtr )
{
    if ( _program )
    {
        // use fragment shader and setup uniforms
        EQ_GL_CALL( glUseProgram( _program ));
        return ;
    }

    // Create fragment shader which reads depth values from 
    // rectangular textures
    const GLuint shader = glCreateShader( GL_FRAGMENT_SHADER );

    EQ_GL_CALL( glShaderSource( shader, 1, &fShaderPtr, 0 ));
    EQ_GL_CALL( glCompileShader( shader ));

    GLint status;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    EQASSERT( status );

    _program = glCreateProgram( );

    EQ_GL_CALL( glAttachShader( _program, shader ));
    EQ_GL_CALL( glLinkProgram( _program ));

    glGetProgramiv( _program, GL_LINK_STATUS, &status );
    EQASSERT( status );

    // use fragment shader and setup uniforms
    EQ_GL_CALL( glUseProgram( _program ));

}


void CompressorYUV::_compress( const GLEWContext* glewContext,
                               const uint64_t inDims[4], uint64_t outDims[4] )
{

    if ( _fbo )
    {
        EQCHECK( _fbo->resize( outDims[1], outDims[3] ));
    }
    else
    {
        _fbo = new util::FrameBufferObject( glewContext );
        _fbo->setColorFormat( GL_RGBA );
        _fbo->init( outDims[1], outDims[3], 0, 0 );
    }

    _fbo->bind();
    _texture->bind();

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, 
                     GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                     GL_NEAREST);

    _init( glewContext, yuv420readback_glsl.c_str() );

    const GLint colorParam = glGetUniformLocation( _program, "color" );
    EQ_GL_CALL( glUniform1i( colorParam, 0 ));

    glDisable( GL_DEPTH_TEST );
    glBegin( GL_QUADS );
        glVertex3i(     0,     0, 0 );
        glVertex3i( outDims[1],     0, 0 );
        glVertex3i( outDims[1], outDims[3], 0 );
        glVertex3i(     0, outDims[3], 0 );
    glEnd();

    // restore state
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));

    _fbo->unbind();
}

void CompressorYUV::_download( void* datas )
{
   _fbo->getColorTextures()[0]->download( datas, GL_RGBA, GL_UNSIGNED_BYTE );
}

void CompressorYUV::download( const GLEWContext* glewContext,
                              const uint64_t  inDims[4],
                              const unsigned  source,
                              const uint64_t  flags,
                              uint64_t        outDims[4],
                              void**          out )
{
    glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT |
                  GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
    glColorMask( true, true, true, true );
    outDims[0] = inDims[0];
    outDims[1] = (inDims[1] + 1) / 2;
    outDims[2] = inDims[2];
    outDims[3] = (inDims[3] + 1) / 2;
    outDims[3] *= 2;
    // first time we instanciate the working texture
    if ( !_texture )
    {
        _texture = new util::Texture( glewContext );
        _texture->setInternalFormat( GL_RGBA );   
    }
    // the data location are in the frame buffer
    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        // read data in frame Buffer
        // compress data 
        _texture->copyFromFrameBuffer( inDims );
        _compress( glewContext, inDims, outDims );
        buffer.resize( outDims[1] * outDims[3] * 4 );
        _download( buffer.getData() );
    }
    // the data location are in the texture id define by
    // the field source
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE )
    {
        // assign texture id to the local texture class
        // compress Data
        // allow buffer memory on cpu
        // transfer data from gpu to cpu
        _texture->setGLData( source, inDims[1], inDims[3] );
        _compress( glewContext, inDims, outDims );
        buffer.resize( outDims[1] * outDims[3] * 4 );
        _download( buffer.getData() );
        _texture->flushNoDelete();
    }
    // no data to transfert, it's only an allocation buffer
    else
    {
        EQUNIMPLEMENTED;
    }
    out[0] = buffer.getData();
    glPopAttrib();
}

void CompressorYUV::_decompress( const GLEWContext* glewContext,
                                 const uint64_t inDims[4],
                                 const uint64_t outDims[4] )
{
    glDepthMask( false );
    _init( glewContext, yuv420unpack_glsl.c_str() );

    const GLint colorParam = glGetUniformLocation( _program, "color" );
    glUniform1i( colorParam, 0 );
    
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
                     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                     GL_CLAMP_TO_EDGE );

    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                         GL_NEAREST );
    
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                         GL_NEAREST );

    glColor3f( 1.0f, 1.0f, 1.0f );
    const float startX = static_cast< float >( inDims[0] );
    const float endX   = static_cast< float >( inDims[1] ) + startX;
    const float startY = static_cast< float >( inDims[2] );
    const float endY   = static_cast< float >( inDims[3] ) + startY;

    const GLint shiftX = glGetUniformLocation( _program, "shiftX" );
    glUniform1f( shiftX, startX );
    const GLint shiftY = glGetUniformLocation( _program, "shiftY" );
    glUniform1f( shiftY, startY );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( startX, startY, 0.0f );

        glTexCoord2f( static_cast< float >( inDims[1] ), 0.0f );
        glVertex3f( endX, startY, 0.0f );

        glTexCoord2f( static_cast<float>( inDims[1] ), 
                      static_cast<float>( inDims[3] ));
        glVertex3f( endX, endY, 0.0f );
        
        glTexCoord2f( 0.0f, static_cast< float >( inDims[3] ));
        glVertex3f( startX, endY, 0.0f );
    glEnd();

    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));
    glDepthMask( true );
}

void CompressorYUV::upload( const GLEWContext* glewContext, 
                            const void*     datas,
                            const uint64_t  inDims[4],
                            const uint64_t  flags,
                            const uint64_t  outDims[4],  
                            const unsigned  destination )
{
    glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT |
                  GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
    //glColorMask( true, true, true, true );
    if ( !_texture )
    {
        _texture = new util::Texture( glewContext );
        _texture->setInternalFormat( GL_RGBA );
    }
    
    if ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {    
        _texture->upload( inDims[1], inDims[3], const_cast<void*>( datas ) );
        _decompress( glewContext, outDims, inDims );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE  )
    {
  
        if ( !_fbo )
        {
            _fbo = new util::FrameBufferObject( glewContext );
            _fbo->setColorFormat( GL_RGBA );
        }

        util::Texture* texture = _fbo->getColorTextures().front();
        texture->setGLData( destination, outDims[1], outDims[3] );
        if ( _fbo->isValid() )
        {
            _fbo->bind();
            texture->bindToFBO( GL_COLOR_ATTACHMENT0 , outDims[1], outDims[3] );
        }
        else
            _fbo->init( outDims[1], outDims[3], 0, 0 );

        _texture->upload( inDims[1], inDims[3], datas );
        _decompress( glewContext, outDims, inDims );
        _fbo->unbind();
        texture->flushNoDelete();
    }
    glPopAttrib();
}

}
}
