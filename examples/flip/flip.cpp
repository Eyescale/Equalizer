
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <eq/eq.h>

#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }

class Node : public eq::Node
{
public:
    virtual bool init()
        {
            EQINFO << "Init " << this << endl;
            return true;
        }

    virtual void exit()
        {
            EQINFO << "Exit " << this << endl;
        }
};

class Channel : public eq::Channel
{
public:
    Channel() : _spin(0.) {}

    virtual bool init()
        {
            cout << "Init " << this << endl;
            return true;
        }

    virtual void exit()
        {
            cout << "Exit " << this << endl;
        }

    virtual void draw()
        {
            applyBuffer();
            applyViewport();
            
            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            applyFrustum();
            
            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();
            applyHeadTransform();
            
            glTranslatef( 0, 0, -2 );
            glRotatef( _spin, 0, 0, 1. );
            _spin += .1;

            glColor3f( 1, 1, 0 );
            glBegin( GL_TRIANGLE_STRIP );
            glVertex3f( -.25, -.25, -.25 );
            glVertex3f( -.25,  .25, -.25 );
            glVertex3f(  .25, -.25, -.25 );
            glVertex3f(  .25,  .25, -.25 );
            glEnd();
            glFinish();
        }
private:
    float _spin;
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual Node*    createNode()    { return new ::Node; }
    virtual Channel* createChannel() { return new ::Channel; }
};

eq::NodeFactory* eq::createNodeFactory()
{
    return new ::NodeFactory;
}

int main( int argc, char** argv )
{
    if( !eq::init( argc, argv ))
        abort();

    eq::Server     server;
    eq::OpenParams openParams;
    openParams.address = "localhost:4242";
    openParams.appName = "foo";

    if( !server.open( openParams ))
        DIE("Can't open server.");

    eq::ConfigParams configParams;
    eq::Config*      config = server.chooseConfig( configParams );

    if( !config )
        DIE("No matching config on server.");

    if( !config->init( ))
        DIE("Config initialisation failed.");

    while( true )
    {
        // update database

        config->frameBegin();
//         config->renderData(...);
//         ...;
        config->frameEnd();

        // process events
    }

    sleep( 5 );
    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    return EXIT_SUCCESS;
}

