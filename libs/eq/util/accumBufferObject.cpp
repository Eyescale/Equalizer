
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2009, Sarah Amsellem <sarah.amsellem@gmail.com>
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

namespace eq
{
namespace util
{

AccumBufferObject::AccumBufferObject( const GLEWContext* glewContext )
    : FrameBufferObject( glewContext )
    , _texture( 0 )
    , _pvp( 0, 0, 0, 0 )
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

    if( FrameBufferObject::init( pvp.w, pvp.h, GL_RGBA32F, 0, 0 ))
    {
        unbind();
        return true;
    }

    exit();
    return false;
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

    bind();
    _drawQuadWithTexture( _texture, 
                          fabric::PixelViewport( 0, 0, getWidth(), getHeight()),
                          value );
    unbind();
    EQ_GL_ERROR( "after AccumBufferObject::load" );
}

void AccumBufferObject::accum( const GLfloat value )
{
    _texture->copyFromFrameBuffer( _texture->getInternalFormat(), _pvp );

    bind();
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );

    _drawQuadWithTexture( _texture,
                          fabric::PixelViewport( 0, 0, getWidth(), getHeight()),
                          value );

    glBlendFunc( GL_ONE, GL_ZERO );
    glDisable( GL_BLEND );
    unbind();
}

void AccumBufferObject::display( const GLfloat value )
{
    _drawQuadWithTexture( getColorTextures()[0], _pvp, value );
}

void AccumBufferObject::_drawQuadWithTexture( Texture* texture, 
                                              const PixelViewport& pvp, 
                                              const GLfloat value )
{
    texture->bind();

    glDepthMask( false );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    texture->applyWrap();
    texture->applyZoomFilter( FILTER_NEAREST );

    glColor3f( value, value, value );

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
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDepthMask( true );
}

}
}

