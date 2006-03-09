
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <eq/eq.h>

#include <stdlib.h>

using namespace std;

#define DIE(reason)    { cout << (reason) << endl; abort(); }

enum DataType
{
    OBJECT_FRAMEDATA = eqNet::MOBJECT_CUSTOM
};

class FrameData : public eqNet::VersionedObject
{
public:
    FrameData() : VersionedObject( OBJECT_FRAMEDATA ), spin(0.) {}

    FrameData( const void* data, const uint64_t size ) 
            : VersionedObject( OBJECT_FRAMEDATA )
        {
            EQASSERT( size == sizeof( spin ));
            spin = *(float*)data;
        }

    float spin;

protected:
    const void* getInstanceData( uint64_t* size )
        { return pack( size ); }

    const void* pack( uint64_t* size )
        {
            *size = sizeof( spin );
            return &spin;
        }

    void unpack( const void* data, const uint64_t size )
        {
            EQASSERT( size == sizeof( spin ));
            spin = *(float*)data;
        }
};


class Config : public eq::Config
{
protected:
    eqNet::Mobject* instanciateMobject( const uint32_t type, const void* data, 
                                        const uint64_t dataSize )
        {
            if( type == OBJECT_FRAMEDATA )
                return new FrameData( data, dataSize );

            return eqNet::Session::instanciateMobject( type, data, dataSize );
        }
};

class Channel : public eq::Channel
{
public:
    Channel() : _frameData(NULL) {}

    virtual bool init( const uint32_t initID )
        {
            cout << "Init channel initID " << initID << " ptr " << this << endl;
            _frameData = (FrameData*)getConfig()->getMobject( initID );
            EQASSERT( _frameData );
            return true;
        }

    virtual void exit()
        {
            cout << "Exit " << this << endl;
        }

    virtual void draw( const uint32_t frameID )
        {
            applyBuffer();
            applyViewport();
            
            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            applyFrustum();
            
            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();
            applyHeadTransform();
            
            _frameData->sync( frameID );

            glTranslatef( 0, 0, -3 );
            glRotatef( _frameData->spin, 0, 0, 1. );

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
    FrameData* _frameData;
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual Config*  createConfig()  { return new ::Config; }
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

    FrameData frameData;
    config->registerMobject( &frameData, config->getNode( ));

    eqBase::Clock clock;
    if( !config->init( frameData.getID( )))
        DIE("Config initialisation failed.");
    cerr << "Config init took " << clock.getTimef() << " ms" << endl;

    int nFrames = 100;
    clock.reset();
    while( nFrames-- )
    {
        // update database
        frameData.spin += .1;
        const uint32_t version = frameData.commit();

        config->frameBegin( version );
//         config->renderData(...);
//         ...;
        config->frameEnd();

        // process events
    }
    cerr << "Rendering took " << clock.getTimef() << " ms" << endl;

    //sleep( 5 );
    clock.reset();
    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    cerr << "Exit took " << clock.getTimef() << " ms" << endl;
    return EXIT_SUCCESS;
}

