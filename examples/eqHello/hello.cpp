
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * Equalizer 'Hello, World!' example. Shows the minimum Equalizer program,
 * rendering spinning quads around the origin.
 */

#include <eq/eq.h>
#include <stdlib.h>

using namespace eq::base;
using namespace std;

namespace eqHello
{
class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent ) : eq::Channel( parent ) {}

protected:
    virtual void frameDraw( const uint32_t spin );
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new Channel( parent ); }
};
}

int main( const int argc, char** argv )
{
    // 1. Equalizer initialization
    eqHello::NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }
    
    // 2. get a configuration
    bool        error  = false;
    eq::Config* config = eq::getConfig( argc, argv );
    if( config )
    {
        // 3. init config
        if( config->init( 0 ))
        {
            // 4. run main loop
            uint32_t spin = 0;
            while( config->isRunning( ))
            {
                config->startFrame( ++spin );
                config->finishFrame();
            }
        
            // 5. exit config
            config->exit();
        }
        else
            error = true;

        // 6. release config
        eq::releaseConfig( config );
    }
    else
    {
        EQERROR << "Cannot get config" << endl;
        error = true;
    }    

    // 7. exit
    eq::exit();
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

/** The rendering routine, a.k.a., glutDisplayFunc() */
void eqHello::Channel::frameDraw( const uint32_t spin )
{
    // setup OpenGL State
    eq::Channel::frameDraw( spin );
    
    const float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );

    // rotate scene around the origin
    glRotatef( static_cast< float >( spin ) * 0.1f, 1.0f, 0.5f, 0.25f );

    // render six axis-aligned colored quads around the origin
    //  front
    glColor3f( 1.0f, 0.5f, 0.5f );
    glNormal3f( 0.0f, 0.0f, 1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, -1.0f );
    glVertex3f( -.7f,  .7f, -1.0f );
    glVertex3f(  .7f, -.7f, -1.0f );
    glVertex3f( -.7f, -.7f, -1.0f );
    glEnd();

    //  bottom
    glColor3f( 0.5f, 1.0f, 0.5f );
    glNormal3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, -1.0f,  .7f );
    glVertex3f( -.7f, -1.0f,  .7f );
    glVertex3f(  .7f, -1.0f, -.7f );
    glVertex3f( -.7f, -1.0f, -.7f );
    glEnd();

    //  back
    glColor3f( 0.5f, 0.5f, 1.0f );
    glNormal3f( 0.0f, 0.0f, -1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, 1.0f );
    glVertex3f( -.7f,  .7f, 1.0f );
    glVertex3f(  .7f, -.7f, 1.0f );
    glVertex3f( -.7f, -.7f, 1.0f );
    glEnd();

    //  top
    glColor3f( 1.0f, 1.0f, 0.5f );
    glNormal3f( 0.f, -1.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, 1.0f,  .7f );
    glVertex3f( -.7f, 1.0f,  .7f );
    glVertex3f(  .7f, 1.0f, -.7f );
    glVertex3f( -.7f, 1.0f, -.7f );
    glEnd();

    //  right
    glColor3f( 1.0f, 0.5f, 1.0f );
    glNormal3f( -1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( 1.0f,  .7f,  .7f );
    glVertex3f( 1.0f, -.7f,  .7f );
    glVertex3f( 1.0f,  .7f, -.7f );
    glVertex3f( 1.0f, -.7f, -.7f );
    glEnd();

    //  left
    glColor3f( 0.5f, 1.0f, 1.0f );
    glNormal3f( 1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -1.0f,  .7f,  .7f );
    glVertex3f( -1.0f, -.7f,  .7f );
    glVertex3f( -1.0f,  .7f, -.7f );
    glVertex3f( -1.0f, -.7f, -.7f );
    glEnd();
}
