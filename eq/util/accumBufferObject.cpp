/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

#include "accumBufferObject.h"

#include <eq/gl.h>

namespace eq
{
namespace util
{

AccumBufferObject::AccumBufferObject( const GLEWContext* glewContext )
    : FrameBufferObject( glewContext )
    , _texture( 0 )
    , _pvp( 0, 0, 0, 0 )
    , _previousFBO( 0 )
{
}

AccumBufferObject::~AccumBufferObject()
{
    exit();
}

bool AccumBufferObject::init( const PixelViewport& pvp,
                              const GLuint textureFormat )
{
    _pvp = pvp;
    _texture = new Texture( GL_TEXTURE_RECTANGLE_ARB, glewGetContext( ));
    _texture->init( textureFormat, pvp.w, pvp.h );

    const Error error = FrameBufferObject::init( pvp.w, pvp.h,
                                                 GL_RGBA32F, 0, 0 );
    if( error )
    {
        LBDEBUG << "FrameBufferObject init failed: " << error << std::endl;
        exit();
        return false;
    }

    unbind();
    return true;
}

void AccumBufferObject::exit()
{
    if( _texture )
        _texture->flush();

    delete _texture;
    _texture = 0;

    FrameBufferObject::exit();
}

void AccumBufferObject::load( const GLfloat value )
{
    EQ_GL_ERROR( "before AccumBufferObject::load" );
    _texture->copyFromFrameBuffer( _texture->getInternalFormat(), _pvp );

    const PixelViewport pvp( 0, 0, getWidth(), getHeight( ));
    _setup( pvp );
    _drawQuadWithTexture( _texture, pvp, value );
    _reset();

    EQ_GL_ERROR( "after AccumBufferObject::load" );

#if 0
    static a_int32_t i;
    std::ostringstream os;
    os << "abo" << ++i;
    getColorTextures()[0]->writeRGB( os.str( ));

    os << "tex";
    _texture->writeRGB( os.str( ));
#endif
}

void AccumBufferObject::accum( const GLfloat value )
{
    _texture->copyFromFrameBuffer( _texture->getInternalFormat(), _pvp );

    const PixelViewport pvp( 0, 0, getWidth(), getHeight( ));
    _setup( pvp );
    EQ_GL_CALL( glEnable( GL_BLEND ));
    EQ_GL_CALL( glBlendFunc( GL_ONE, GL_ONE ));

    _drawQuadWithTexture( _texture, pvp, value );

    EQ_GL_CALL( glBlendFunc( GL_ONE, GL_ZERO ));
    EQ_GL_CALL( glDisable( GL_BLEND ));
    _reset();
}

void AccumBufferObject::display( const GLfloat value )
{
    _drawQuadWithTexture( getColorTextures()[0], _pvp, value );
}

bool AccumBufferObject::resize( const PixelViewport& pvp )
{
    if( _pvp == pvp )
        return false;

    _pvp = pvp;
    return FrameBufferObject::resize( pvp.w, pvp.h );
}

void AccumBufferObject::_setup( const PixelViewport& pvp )
{
    EQ_GL_CALL( glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &_previousFBO ));
    bind();
    EQ_GL_CALL( glPushAttrib( GL_SCISSOR_BIT | GL_VIEWPORT_BIT |
                              GL_TRANSFORM_BIT ));
    EQ_GL_CALL( glMatrixMode(GL_PROJECTION));
    EQ_GL_CALL( glPushMatrix());
    EQ_GL_CALL( glLoadIdentity());
    EQ_GL_CALL( glOrtho(0, pvp.w, 0, pvp.h, -1, 1));
    EQ_GL_CALL( glScissor(0, 0, pvp.w, pvp.h));
    EQ_GL_CALL( glViewport(0, 0, pvp.w, pvp.h));
}

void AccumBufferObject::_reset()
{
    EQ_GL_CALL( glMatrixMode(GL_PROJECTION));
    EQ_GL_CALL( glPopMatrix());
    EQ_GL_CALL( glPopAttrib());
    EQ_GL_CALL( glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _previousFBO ));
}

void AccumBufferObject::_drawQuadWithTexture( Texture* texture,
                                              const PixelViewport& pvp,
                                              const GLfloat value )
{
    texture->bind();

    EQ_GL_CALL( glDepthMask( false ));
    EQ_GL_CALL( glDisable( GL_LIGHTING ));
    EQ_GL_CALL( glEnable( GL_TEXTURE_RECTANGLE_ARB ));
    EQ_GL_CALL( glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ));
    texture->applyWrap();
    texture->applyZoomFilter( FILTER_NEAREST );

    EQ_GL_CALL( glColor4f( value, value, value, value ));

    const float startX = static_cast< float >( pvp.x );
    const float endX   = static_cast< float >( pvp.x + pvp.w );
    const float startY = static_cast< float >( pvp.y );
    const float endY   = static_cast< float >( pvp.y + pvp.h );

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( startX, startY, 0.0f );

        glTexCoord2f( static_cast< float >( pvp.w ), 0.0f );
        glVertex3f( endX, startY, 0.0f );

        glTexCoord2f( static_cast<float>( pvp.w ), static_cast<float>( pvp.h ));
        glVertex3f( endX, endY, 0.0f );

        glTexCoord2f( 0.0f, static_cast< float >( pvp.h ));
        glVertex3f( startX, endY, 0.0f );
    glEnd();

    // restore state
    EQ_GL_CALL( glDisable( GL_TEXTURE_RECTANGLE_ARB ));
    EQ_GL_CALL( glDepthMask( true ));
}

}
}
