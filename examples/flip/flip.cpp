
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
            INFO << "Init " << this << endl;
            return true;
        }

    virtual void exit()
        {
            INFO << "Exit " << this << endl;
        }
};

class Channel : public eq::Channel
{
public:
    virtual bool init()
        {
            cout << "Init " << this << endl;
            return true;
        }

    virtual void exit()
        {
            cout << "Exit " << this << endl;
        }
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

    eq::Server server;
    string address = "localhost:4242";
    if( !server.open( address ))
        DIE("Can't open server.");

    eq::ConfigParams params;
    params.appName = "foo";

    eq::Config* config = server.chooseConfig( &params );
    if( !config )
        DIE("No matching config on server.");

    if( !config->init( ))
        DIE("Config initialisation failed.");

    //while( running )
    {
        // update database

        config->frameBegin();
//         config->renderData(...);
//         ...;
        config->frameEnd();

        // process events
    }

    sleep( 1 );
    config->exit();
    server.releaseConfig( config );
    server.close();
    eq::exit();
    return EXIT_SUCCESS;
}

