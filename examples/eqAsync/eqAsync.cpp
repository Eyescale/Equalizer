
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *                    2012, Stefan Eilemann <eile@eyescale.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "eqAsync.h"

namespace eqAsync
{

bool Window::configInitGL( const eq::uint128_t& initID )
{
    if( !eq::Window::configInitGL( initID ))
        return false;

    Pipe* pipe = static_cast<Pipe*>( getPipe( ));
    pipe->startAsyncFetcher( this );
    return true;
}

void Pipe::startAsyncFetcher( Window* wnd )
{
    if( _initialized )
        return;
    _initialized = true;
    LBINFO << "initialize async fetcher: " << this << ", " << wnd << std::endl;
    _asyncFetcher.setup( wnd );
}


void Pipe::frameStart( const eq::uint128_t& frameID, const uint32_t frameNumber)
{
    eq::Pipe::frameStart( frameID, frameNumber );

    const void* oldKey = _txId.key;
    if( _asyncFetcher.tryGetTextureId( _txId ))
    {
        if( oldKey != 0 )
            _asyncFetcher.deleteTexture( oldKey );

        LBINFO << "new texture generated " << _txId.key << std::endl;
    }
}

bool Pipe::configExit()
{
    _asyncFetcher.stop();
    return eq::Pipe::configExit();
}

void Channel::frameDraw( const eq::uint128_t& spin )
{
    // setup OpenGL State
    eq::Channel::frameDraw( spin );

    Pipe*  pipe = static_cast< Pipe* >( getPipe( ));
    GLuint txId = pipe->getTextureId();

    if( txId )
    {
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, txId );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    }

    const float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );

    // rotate scene around the origin
    glRotatef( static_cast< float >( spin.low( )) * 0.1f, 1.0f, 0.5f, 0.25f );

    float tMin = 0.f;
    float tMax = 1.f;
    // render six axis-aligned colored quads around the origin
    for( int i = 0; i < 6; i++ )
    {
        glColor3f( i&1 ? 0.5f : 1.0f, i&2 ? 1.0f : 0.5f, i&4 ? 1.0f : 0.5f );

        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_TRIANGLE_STRIP );
            glTexCoord2f( tMax, tMax );
            glVertex3f(  .7f,  .7f, -1.0f );
            glTexCoord2f( tMin, tMax );
            glVertex3f( -.7f,  .7f, -1.0f );
            glTexCoord2f( tMax, tMin );
            glVertex3f(  .7f, -.7f, -1.0f );
            glTexCoord2f( tMin, tMin );
            glVertex3f( -.7f, -.7f, -1.0f );
        glEnd();

        if( i < 3 )
            glRotatef(  90.0f, 0.0f, 1.0f, 0.0f );
        else if( i == 3 )
            glRotatef(  90.0f, 1.0f, 0.0f, 0.0f );
        else
            glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
    }

    if( txId )
    {
        glDisable( GL_TEXTURE_2D );
    }
}

} // namespace eqAsync
