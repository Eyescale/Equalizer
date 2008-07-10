
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// Tests the functionality of the config visitor

#include <test.h>
#include <eq/client/channel.h>

using namespace eq;

class TestVisitor : public ConfigVisitor
{
public:
    TestVisitor() : nConfigs(0), nNodes(0), nPipes(0), nWindows(0), nChannels(0)
        {}

    virtual Result visit( Config* config )
        {
            ++nConfigs;
            return TRAVERSE_CONTINUE;
        }

    virtual Result visit( Node* node )
        {
            ++nNodes;
            if( nNodes == 1 )
                return TRAVERSE_PRUNE;
            return TRAVERSE_CONTINUE;
        }

    virtual Result visit( Pipe* pipe )
        {
            ++nPipes;
            if( nPipes == 5 )
                return TRAVERSE_TERMINATE;
            return TRAVERSE_CONTINUE;
        }

    virtual Result visit( eq::Window* window )
        {
            ++nWindows;
            return TRAVERSE_CONTINUE;
        }

    virtual Result visit( Channel* channel )
        {
            ++nChannels;
            return TRAVERSE_PRUNE; 
        }

    size_t nConfigs;
    size_t nNodes;
    size_t nPipes;
    size_t nWindows;
    size_t nChannels;
};

int main( int argc, char **argv )
{
    // setup
    base::RefPtr< Client > client = new Client;
    TEST( client->initLocal( argc, argv ));

    base::RefPtr< Server > server = new Server;
    server->setClient( client );

    Config* config = new Config( server );
    client->addSession( config, server.get(), 0, "" );

    for( size_t i = 0; i < 3; ++i )
    {
        Node* node = new Node( config );
        for( size_t j = 0; j < 3; ++j )
        {
            Pipe* pipe = new Pipe( node );
            for( size_t k = 0; k < 3; ++k )
            {
                eq::Window* window = new eq::Window( pipe );
                for( size_t l = 0; l < 3; ++l )
                    new Channel( window );
            }
        }
    }

    // tests
    TestVisitor visitor;
    const TestVisitor::Result result = config->accept( &visitor );

    TESTINFO( result == TestVisitor::TRAVERSE_TERMINATE, result );
    TESTINFO( visitor.nConfigs == 1,   visitor.nConfigs );
    TESTINFO( visitor.nNodes == 3,     visitor.nNodes );
    TESTINFO( visitor.nPipes == 5,     visitor.nPipes );
    TESTINFO( visitor.nWindows == 12,  visitor.nWindows );
    TESTINFO( visitor.nChannels == 36, visitor.nChannels );

    // teardown
    TEST( client->exitLocal( ));
    return result;
}
