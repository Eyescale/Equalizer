
/* Copyright (c) 2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <lunchbox/test.h>
#include <eq/eq.h>
#include <eq/admin/admin.h>

class Application
{
public:
    Application( const int argc, char** argv )
        : client( new eq::Client )
        , server( new eq::Server )
        , config( 0 )
    {
        TEST( client->initLocal( argc, argv ));
        TEST( client->connectServer( server ));

        eq::fabric::ConfigParams configParams;
        config = server->chooseConfig( configParams );
        if( config ) // Autoconfig failed, likely because there are no GPUs
            TEST( config->init( co::uint128_t( )));
    }

    ~Application()
    {
        if( config ) // Autoconfig failed, likely because there are no GPUs
        {
            TEST( config->exit( ));
            server->releaseConfig( config );
        }

        TEST( client->disconnectServer( server ));
        TEST( client->exitLocal( ));
        TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
        TESTINFO( server->getRefCount() == 1, server->getRefCount( ));
    }

    eq::ClientPtr client;
    eq::ServerPtr server;
    eq::Config* config;
};

class Admin
{
public:
    Admin( const int argc, char** argv )
        : client( new eq::admin::Client )
        , server( new eq::admin::Server )
        , config( 0 )
        , window( 0 )
        , channel( 0 )
        , canvas( 0 )
        , segment( 0 )
        , layout( 0 )
        , view( 0 )
    {
        TEST( eq::admin::init( argc, argv ));
        TEST( client->initLocal( argc, argv ));
        TEST( client->connectServer( server ));

        TEST( server->getConfigs().size() == 1 );
        config = server->getConfigs().front();
        TEST( config );
    }

    ~Admin()
    {
        delete view;
        delete segment;
        delete canvas;
        delete layout;
        delete channel;
        delete window;
        config->commit();

        TEST( client->disconnectServer( server ));
        TEST( client->exitLocal( ));
        TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
        TESTINFO( server->getRefCount() == 1, server->getRefCount( ));

        TEST( eq::admin::exit( ));
    }

    void addWindow( const eq::uint128_t& pipeID, const bool letItFail = false )
    {
        eq::admin::Pipe* pipe = config->find< eq::admin::Pipe >( pipeID );
        window = new eq::admin::Window( pipe );
        channel = new eq::admin::Channel( window );
        canvas = new eq::admin::Canvas( config );
        segment = new eq::admin::Segment( canvas );
        layout = new eq::admin::Layout( config );
        view = new eq::admin::View( layout );

        window->setIAttribute( eq::fabric::WindowSettings::IATTR_HINT_DRAWABLE,
                               eq::fabric::FBO );
        if( letItFail )
        {
            window->setIAttribute(
                        eq::fabric::WindowSettings::IATTR_PLANES_STENCIL,
                        5000 );
        }
        window->setPixelViewport( eq::fabric::PixelViewport( 0, 0, 400, 300));
        segment->setChannel( channel );
        canvas->addLayout( layout );

        config->commit();
    }

    eq::admin::ClientPtr client;
    eq::admin::ServerPtr server;
    eq::admin::Config* config;

    eq::admin::Window* window;
    eq::admin::Channel* channel;
    eq::admin::Canvas* canvas;
    eq::admin::Segment* segment;
    eq::admin::Layout* layout;
    eq::admin::View* view;
};

void runTest( const int argc, char** argv )
{
    Application app( argc, argv );
    if( !app.config ) // Autoconfig failed, likely because there are no GPUs
        return;

    TEST( app.config->update( ));

    const eq::Nodes& nodes = app.config->getNodes();
    TEST( nodes.size() == 1 );
    const eq::Node* node = nodes.front();
    const eq::Pipes& pipes = node->getPipes();
    TEST( !pipes.empty( ));
    eq::Pipe* pipe = pipes.front();

    Admin admin( argc, argv );
    admin.addWindow( pipe->getID( ));
    TEST( app.config->update( ));
    TEST( app.config->getErrors().empty( ));

    // let the window creation fail now
    admin.addWindow( pipe->getID(), true );
    TEST( !app.config->update( ));
    TEST( !app.config->getErrors().empty( ));
    TEST( app.config->getErrors().empty( ));
}


int main( const int argc, char** argv )
{
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    runTest( argc, argv );

    TEST( eq::exit( ));

    return EXIT_SUCCESS;
}
