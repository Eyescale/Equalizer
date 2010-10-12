
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

eq::base::a_int32_t drawCalls;
eq::base::a_int32_t readbackCalls;
eq::base::a_int32_t assembleCalls;

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

class Channel : public eq::Channel
{
public:
    Channel( eq::Window* parent ) : eq::Channel( parent ) {}

protected:
    virtual void frameDraw( const uint32_t frameID )
        { eq::Channel::frameDraw( frameID ); ++drawCalls; }
    virtual void frameReadback( const uint32_t frameID )
        { eq::Channel::frameReadback( frameID ); ++readbackCalls; }
    virtual void frameAssemble( const uint32_t frameID )
        { eq::Channel::frameAssemble( frameID ); ++assembleCalls; }
};

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Node* createNode( eq::Config* parent )
        { return new Node( parent ); }
    virtual eq::Pipe* createPipe( eq::Node* parent )
        { return new Pipe( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new Channel( parent ); }
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

    // get number of operations expected
    std::string name = config->getName();
    size_t index = name.find( '-' );
    TESTINFO( index != std::string::npos,
              "Config name has to be '<name>-<int>d<int>r<int>a': " << filename);
    name = name.substr( index + 1 );
    const int nDraw = atoi( name.c_str( ));

    index = name.find( 'd' );
    TESTINFO( index != std::string::npos,
              "Config name has to be '<name>-<int>d<int>r<int>a': " << filename);
    name = name.substr( index + 1 );
    const int nReadback = atoi( name.c_str( ));

    index = name.find( 'r' );
    TESTINFO( index != std::string::npos,
              "Config name has to be '<name>-<int>d<int>r<int>a': " << filename);
    name = name.substr( index + 1 );
    const int nAssemble = atoi( name.c_str( ));

    // 3. init config
    TESTINFO( config->init( 0 ), filename );

    // 4. run main loop
    config->startFrame( 0 );
    config->finishFrame();
    config->startFrame( 0 );
    config->finishFrame();
    config->startFrame( 0 );
    config->finishAllFrames();

    TESTINFO( nDraw == drawCalls,
              filename << ": " << nDraw << " != " << drawCalls );
    TESTINFO( nReadback == readbackCalls,
              filename << ": " << nReadback << " != " << readbackCalls);
    TESTINFO( nAssemble == assembleCalls,
              filename << ": " << nAssemble << " != " << assembleCalls);
    drawCalls = 0;
    readbackCalls = 0;
    assembleCalls = 0;

    // 5. exit config
    TESTINFO( config->exit(), filename );

    // 6. release config
    server->releaseConfig( config );
    client->disconnectServer( server );
}
