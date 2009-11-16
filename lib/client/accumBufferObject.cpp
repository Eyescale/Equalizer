
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *                   , Sarah Amsellem <sarah.amsellem@gmail.com>
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

AccumBufferObject::AccumBufferObject( GLEWContext* glewContext )
    : FrameBufferObject( glewContext )
    , _texture( 0 )
{
}

AccumBufferObject::~AccumBufferObject()
{
    exit();
}

bool AccumBufferObject::init( const int width, const int height,
                              GLuint textureFormat )
{
    _texture = new Texture( glewGetContext( ));
    _texture->setFormat( textureFormat );

    setColorFormat( GL_RGBA32F );
    if( FrameBufferObject::init( width, height, 0, 0 ))
        return true;

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

void AccumBufferObject::load( GLfloat value )
{
    _texture->copyFromFrameBuffer( getPixelViewport( ));

    bind();
    glEnable( GL_BLEND );

    glClear(GL_COLOR_BUFFER_BIT);
    _drawQuadWithTexture( _texture, value );

    glDisable( GL_BLEND );
    unbind();
}

void AccumBufferObject::accum( GLfloat value )
{
    _texture->copyFromFrameBuffer( getPixelViewport( ));

    bind();
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );

    _drawQuadWithTexture( _texture, value );

    glBlendFunc( GL_ONE, GL_ZERO );
    glDisable( GL_BLEND );
    unbind();
}

void AccumBufferObject::display( GLfloat value )
{
    _drawQuadWithTexture( getColorTextures()[0], value );
}

void AccumBufferObject::_drawQuadWithTexture( Texture* texture, GLfloat value )
{
    texture->bind();

    glDepthMask( false );
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

    glColor3f( value, value, value );

    PixelViewport pvp = getPixelViewport();

    const float startX = pvp.x;
    const float endX   = pvp.x + pvp.w;
    const float startY = pvp.y;
    const float endY   = pvp.y + pvp.h;

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

