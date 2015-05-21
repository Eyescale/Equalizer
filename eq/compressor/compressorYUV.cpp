/* Copyright (c)      2009, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2009, Maxim Makhinya
 *               2010-2013, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/gl.h>
#include <eq/zoomFilter.h>
#include <eq/fabric/pixelViewport.h>

#include "yuv420readback_glsl.h"
#include "yuv420unpack_glsl.h"

#define glewGetContext() glewContext

namespace eq
{
namespace plugin
{
namespace
{
static void _getInfo( EqCompressorInfo* const info )
{
    info->version         = EQ_COMPRESSOR_VERSION;
    info->name            = EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUVA_50P;
    info->capabilities    = EQ_COMPRESSOR_TRANSFER | EQ_COMPRESSOR_DATA_2D |
                            EQ_COMPRESSOR_USE_TEXTURE_RECT |
                            EQ_COMPRESSOR_USE_FRAMEBUFFER;
    info->tokenType       = EQ_COMPRESSOR_DATATYPE_RGBA;
    info->outputTokenType = EQ_COMPRESSOR_DATATYPE_YUVA_50P;
    info->outputTokenSize = 4;
    info->quality         = 0.5f;
    info->ratio           = 0.5f;
    info->speed           = 0.5f;
}

static bool _register()
{
    Compressor::registerEngine(
        Compressor::Functions( EQ_COMPRESSOR_TRANSFER_RGBA_TO_YUVA_50P,
                               _getInfo, CompressorYUV::getNewCompressor,
                               CompressorYUV::getNewDecompressor, 0,
                               CompressorYUV::isCompatible ));
    return true;
}

static bool _initialized LB_UNUSED = _register();
}

/** Construct a new compressor Yuv */
CompressorYUV::CompressorYUV()
        : Compressor()
        , _program( 0 )
        , _fbo( 0 )
        , _texture( 0 )
{ }

/** Destruct the compressor Yuv */
CompressorYUV::~CompressorYUV( )
{
    // replace original texture
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
    return ( GLEW_ARB_texture_non_power_of_two &&
             GLEW_VERSION_2_0 &&
             GLEW_EXT_framebuffer_object );
}

void CompressorYUV::_initShader( const GLEWContext* glewContext LB_UNUSED,
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
    LBASSERT( status );

    _program = glCreateProgram( );

    EQ_GL_CALL( glAttachShader( _program, shader ));
    EQ_GL_CALL( glLinkProgram( _program ));

    glGetProgramiv( _program, GL_LINK_STATUS, &status );
    LBASSERT( status );

    // use fragment shader and setup uniforms
    EQ_GL_CALL( glUseProgram( _program ));

}


void CompressorYUV::_compress( const GLEWContext* glewContext,
                               const eq_uint64_t* /*inDims*/,
                               eq_uint64_t outDims[4] )
{
    /* save the current FBO ID for bind it at the end of the compression */
    GLint oldFBO = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &oldFBO );

    if ( _fbo )
    {
        LBCHECK( _fbo->resize( outDims[1], outDims[3] ));
        _fbo->bind();
    }
    else
    {
        _fbo = new util::FrameBufferObject( glewContext );
        LBCHECK( _fbo->init( outDims[1], outDims[3], GL_RGBA, 0, 0 ));
    }

    _texture->bind();

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    _texture->applyZoomFilter( FILTER_NEAREST );
    _texture->applyWrap();

    _initShader( glewContext, yuv420readback_glsl.c_str() );

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
    //glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));

    /* apply the initial fbo */
    EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, oldFBO ));
}

void CompressorYUV::_download( void* data )
{
    util::Texture* texture = _fbo->getColorTextures()[0];
    LBASSERT( texture->getFormat() == GL_RGBA );
    LBASSERT( texture->getType() == GL_UNSIGNED_BYTE );
    texture->download( data );
}

void CompressorYUV::download( const GLEWContext* glewContext,
                              const eq_uint64_t  inDims[4],
                              const unsigned     source,
                              const eq_uint64_t  flags,
                                    eq_uint64_t  outDims[4],
                              void**             out )
{
    glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT |
                  GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
    glColorMask( true, true, true, true );
    outDims[0] = inDims[0];
    outDims[1] = (inDims[1] + 1) / 2;
    outDims[2] = inDims[2];
    outDims[3] = (inDims[3] + 1) / 2;
    outDims[3] *= 2;
    buffer.resize( outDims[1] * outDims[3] * 4 );
    // first time we instanciate the working texture
    if ( !_texture )
        _texture = new util::Texture( GL_TEXTURE_RECTANGLE_ARB, glewContext );

    // the data is in the frame buffer
    if( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
         // read data in frame Buffer
        const eq::fabric::PixelViewport pvp( inDims[0], inDims[2],
                                             inDims[1], inDims[3] );
        _texture->init( GL_RGBA, outDims[1]*2, outDims[3] );
        _texture->copyFromFrameBuffer( GL_RGBA, pvp );

        // compress data
        _compress( glewContext, inDims, outDims );
        buffer.resize( outDims[1] * outDims[3] * 4 );
        _download( buffer.getData( ));
    }
    // the data is in the texture id define by the field "source"
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE_RECT )
    {
        // assign texture id to the local texture class
        // compress Data
        // allow buffer memory on cpu
        // transfer data from gpu to cpu
        _texture->setGLData( source, GL_RGBA, inDims[1], inDims[3] );
        _compress( glewContext, inDims, outDims );
        _download( buffer.getData( ));
        _texture->flushNoDelete();
    }
    else
    {
        LBUNIMPLEMENTED;
    }
    out[0] = buffer.getData();
    glPopAttrib();
}

void CompressorYUV::_decompress( const GLEWContext* glewContext,
                                 const eq_uint64_t inDims[4] )
{
    glDepthMask( false );
    _initShader( glewContext, yuv420unpack_glsl.c_str() );

    const GLint colorParam = glGetUniformLocation( _program, "color" );
    glUniform1i( colorParam, 0 );

    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    _texture->applyWrap();
    _texture->applyZoomFilter( FILTER_NEAREST );

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
        glVertex3f( startX, startY, 0.0f );
        glVertex3f(   endX, startY, 0.0f );
        glVertex3f(   endX,   endY, 0.0f );
        glVertex3f( startX,   endY, 0.0f );
    glEnd();

    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    EQ_GL_CALL( glUseProgram( 0 ));
    glDepthMask( true );
}

void CompressorYUV::upload( const GLEWContext* glewContext,
                            const void*        data,
                            const eq_uint64_t  inDims[4],
                            const eq_uint64_t  flags,
                            const eq_uint64_t  outDims[4],
                            const unsigned     destination )
{
    glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT |
                  GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
    if ( !_texture )
    {
        _texture = new util::Texture( GL_TEXTURE_RECTANGLE_ARB, glewContext );
        _texture->init( GL_RGBA, outDims[1], outDims[3] );
    }

    if ( flags & EQ_COMPRESSOR_USE_FRAMEBUFFER )
    {
        _texture->upload( inDims[1], inDims[3], data );
        _decompress( glewContext, outDims );
    }
    else if( flags & EQ_COMPRESSOR_USE_TEXTURE_RECT  )
    {
        /* save the current FBO ID for bind it at the end of the compression */
        GLint oldFBO = 0;
        glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &oldFBO );

        if ( !_fbo )
            _fbo = new util::FrameBufferObject( glewContext );

        util::Texture* texture = _fbo->getColorTextures().front();
        texture->setGLData( destination, GL_RGBA, outDims[1], outDims[3] );

        if( _fbo->isValid() )
        {
            _fbo->bind();
            texture->bindToFBO( GL_COLOR_ATTACHMENT0, outDims[1], outDims[3] );
        }
        else
        {
            LBCHECK( _fbo->init( outDims[1], outDims[3], GL_RGBA, 0, 0 ));
        }

        _texture->upload( inDims[1], inDims[3], data );
        _decompress( glewContext, outDims );

        /* apply the initial fbo */
        EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, oldFBO ));

        texture->flushNoDelete();
    }
    else
    {
        LBASSERT( 0 );
    }
    glPopAttrib();
}

}
}
