
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *  All rights reserved.
 *
 * Equalizer 'Hello, World!' example. Shows the minimum Equalizer program which
 * renders spinning quads around the origin.
 */

#include <eq/eq.h>
#include <stdlib.h>

using namespace eqBase;
using namespace std;

class Channel : public eq::Channel
{
public:
    virtual void frameDraw( const uint32_t spin );
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Channel* createChannel() { return new Channel; }
};

int main( int argc, char** argv )
{
    // 1. Equalizer initialization
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }
    
    bool error = false;

    // 2. initialization of local client node
    RefPtr< eq::Client > client = new eq::Client;
    if( client->initLocal( argc, argv ))
    {
        // 3. connect to server
        RefPtr<eq::Server> server = new eq::Server;
        if( client->connectServer( server ))
        {
            // 4. choose configuration
            eq::ConfigParams configParams;
            eq::Config* config = server->chooseConfig( configParams );
            if( config )
            {
                // 5. init config
                if( config->init( 0 ))
                {
                    // 6. run main loop
                    uint32_t spin = 0;
                    while( config->isRunning( ))
                    {
                        config->startFrame( ++spin );
                        config->finishFrame();
                    }
                    
                    // 7. exit config
                    config->exit();
                }
                else
                {
                    EQERROR << "Config initialization failed: " 
                            << config->getErrorMessage() << endl;
                    error = true;
                }

                // 8. release config
                server->releaseConfig( config );
            }
            else
            {
                EQERROR << "No matching config on server" << endl;
                error = true;
            }

            // 9. disconnect server
            client->disconnectServer( server );
        }
        else
        {
            EQERROR << "Can't open server" << endl;
            error = true;
        }

        // 10. exit local client node
        client->exitLocal();
    }
    else
    {
        EQERROR << "Can't init local client node" << endl;
        error = true;
    }

    // 11. exit
    eq::exit();
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

/** The rendering routine, a.k.a., glutDisplayFunc() */
void Channel::frameDraw( const uint32_t spin )
{
    // setup OpenGL State
    eq::Channel::frameDraw( spin );
    
    const float lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    const float lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv( GL_LIGHT0, GL_AMBIENT, lightAmbient );

    // rotate scene around the origin
    glRotatef( static_cast< float >( spin ) * 0.5f, 1.0f, 0.5f, 0.25f );

    // render six axis-aligned colored quads around the origin
    //  front
    glColor3f( 1.0f, 0.5f, 0.5f );
    glNormal3f( 0.0f, 0.0f, 1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, -1.0f );
    glVertex3f(  .7f, -.7f, -1.0f );
    glVertex3f( -.7f,  .7f, -1.0f );
    glVertex3f( -.7f, -.7f, -1.0f );
    glEnd();

    //  bottom
    glColor3f( 0.5f, 1.0f, 0.5f );
    glNormal3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, -1.0f,  .7f );
    glVertex3f(  .7f, -1.0f, -.7f );
    glVertex3f( -.7f, -1.0f,  .7f );
    glVertex3f( -.7f, -1.0f, -.7f );
    glEnd();

    //  back
    glColor3f( 0.5f, 0.5f, 1.0f );
    glNormal3f( 0.0f, 0.0f, -1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f,  .7f, 1.0f );
    glVertex3f(  .7f, -.7f, 1.0f );
    glVertex3f( -.7f,  .7f, 1.0f );
    glVertex3f( -.7f, -.7f, 1.0f );
    glEnd();

    //  top
    glColor3f( 1.0f, 1.0f, 0.5f );
    glNormal3f( 0.f, -1.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f(  .7f, 1.0f,  .7f );
    glVertex3f(  .7f, 1.0f, -.7f );
    glVertex3f( -.7f, 1.0f,  .7f );
    glVertex3f( -.7f, 1.0f, -.7f );
    glEnd();

    //  right
    glColor3f( 1.0f, 0.5f, 1.0f );
    glNormal3f( -1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( 1.0f,  .7f,  .7f );
    glVertex3f( 1.0f,  .7f, -.7f );
    glVertex3f( 1.0f, -.7f,  .7f );
    glVertex3f( 1.0f, -.7f, -.7f );
    glEnd();

    //  left
    glColor3f( 0.5f, 1.0f, 1.0f );
    glNormal3f( 1.f, 0.f, 0.f );
    glBegin( GL_TRIANGLE_STRIP );
    glVertex3f( -1.0f,  .7f,  .7f );
    glVertex3f( -1.0f,  .7f, -.7f );
    glVertex3f( -1.0f, -.7f,  .7f );
    glVertex3f( -1.0f, -.7f, -.7f );
    glEnd();
}
