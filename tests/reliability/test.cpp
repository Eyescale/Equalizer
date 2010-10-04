
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <test.h>
#include <eq/eq.h>

class Node : public eq::Node
{
public:
    Node( eq::Config* parent ) : eq::Node( parent ) {}

protected:
    virtual bool configInit( const uint32_t initID )
        {
            if( getName() != "fail" )
                return eq::Node::configInit( initID );
            setErrorMessage( "Node::configInit failure" );
            return false;
        }
};

class Pipe : public eq::Pipe
{
public:
    Pipe( eq::Node* parent ) : eq::Pipe( parent ) {}

protected:
    virtual bool configInit( const uint32_t initID )
        {
            if( getName() != "fail" )
                return eq::Pipe::configInit( initID );
            setErrorMessage( "Pipe::configInit failure" );
            return false;
        }
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Node* createNode( eq::Config* parent )
        { return new Node( parent ); }
    virtual eq::Pipe* createPipe( eq::Node* parent )
        { return new Pipe( parent ); }
};

void _testConfig( eq::ClientPtr client, const std::string& filename );

int main( const int argc, char** argv )
{
    // 1. Equalizer initialization
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << std::endl;
        return EXIT_FAILURE;
    }
    
    // 2. get a configuration
    eq::ClientPtr client = new eq::Client;
    eq::net::ConnectionDescriptionPtr desc = new eq::net::ConnectionDescription;
    std::string addr( "127.0.0.1:37490" );
    TEST( desc->fromString( addr ));
    client->addConnectionDescription( desc );
    TEST( client->initLocal( argc, argv ));

    eq::base::Strings configs = eq::base::searchDirectory( ".", "*.eqc" );
    stde::usort( configs ); // have a predictable order

    for( eq::base::Strings::const_iterator i = configs.begin();
        i != configs.end(); ++i )
    {
        const std::string& config = "./" + *i;
        _testConfig( client, config );
    }

    // 7. exit
    client->exitLocal();
    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));

    eq::exit();
    return EXIT_SUCCESS;
}

void _testConfig( eq::ClientPtr client, const std::string& filename )
{
    eq::ServerPtr server = new eq::Server;
    eq::Global::setConfigFile( filename );
    TEST( client->connectServer( server ));

    eq::ConfigParams configParams;
    eq::Config* config = server->chooseConfig( configParams );
    TEST( config );

    // 3. init config
    TEST( config->init( 0 ));

    // 4. run main loop
    config->startFrame( 0 );
    config->finishFrame();
    config->startFrame( 0 );
    config->finishFrame();
    config->startFrame( 0 );
    config->finishFrame();

    // 5. exit config
    TEST( config->exit( ));

    // 6. release config
    server->releaseConfig( config );
    client->disconnectServer( server );
}
