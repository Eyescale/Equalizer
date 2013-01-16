
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests the functionality of the config visitor

#define EQ_IGNORE_GLEW // just to test that somewhere...
#include <test.h>

#include <eq/client/channel.h>
#include <eq/client/client.h>
#include <eq/client/config.h>
#include <eq/client/init.h>
#include <eq/client/node.h>
#include <eq/client/nodeFactory.h>
#include <eq/client/pipe.h>
#include <eq/client/server.h>
#include <eq/client/window.h>
#include <eq/fabric/configVisitor.h>

using namespace eq;

class TestVisitor : public ConfigVisitor
{
public:
    TestVisitor() : nConfigs(0), nNodes(0), nPipes(0), nWindows(0), nChannels(0)
        {}
    virtual ~TestVisitor() {}

    virtual VisitorResult visitPre( Config* config )
        {
            ++nConfigs;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( Node* node )
        {
            ++nNodes;
            if( nNodes == 1 )
                return TRAVERSE_PRUNE;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( Pipe* pipe )
        {
            ++nPipes;
            if( nPipes == 5 )
                return TRAVERSE_TERMINATE;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( eq::Window* window )
        {
            ++nWindows;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visit( Channel* channel )
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
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    for( size_t c = 0; c < 10; ++c )
    {
        eq::ClientPtr client = new Client;
        TEST( client->getRefCount() == 1 );
        TEST( client->initLocal( argc, argv ));

        eq::ServerPtr server = new eq::Server;
        TEST( server->getRefCount() == 1 );
        TEST( client->connectServer( server ));

        Config* config = new Config( server );
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
        const VisitorResult result = config->accept( visitor );

        TESTINFO( result == TRAVERSE_TERMINATE, result );
        TESTINFO( visitor.nConfigs == 1,   visitor.nConfigs );
        TESTINFO( visitor.nNodes == 3,     visitor.nNodes );
        TESTINFO( visitor.nPipes == 5,     visitor.nPipes );
        TESTINFO( visitor.nWindows == 12,  visitor.nWindows );
        TESTINFO( visitor.nChannels == 36, visitor.nChannels );

        // cleanup and exit
        server->releaseConfig( config );
        TEST( client->disconnectServer( server ));
        // teardown
        TEST( client->exitLocal( ));
    }
    return EXIT_SUCCESS;
}
