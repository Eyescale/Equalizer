
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch>
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
#include <eq/fabric/channel.h>

#include <string>
#include <iostream>
#include <cstdio>

#include "../../eq/server/global.h"

static const std::string PREFIX = "prefix_";
static const std::string GENERATED_FILENAME = PREFIX + "1.rgb";

void setupDumpImage();
void deleteFile( const std::string& filename );
bool verifyFileExists( const std::string& filename );
void tearDown( eq::ServerPtr& server, eq::ClientPtr& client,
			   eq::Config* config);

/*
 * Tests the ability to dump views to a file.
 */
int main( const int argc, char** argv )
{
	// 1.- Prepare test
	setupDumpImage();

	// 2.- Start application
    eq::NodeFactory nodeFactory;
    TEST( eq::init( argc, argv, &nodeFactory ));

    eq::ClientPtr client = new eq::Client;
    TEST( client->initLocal( argc, argv ));

    eq::ServerPtr server = new eq::Server;
    TEST( client->connectServer( server ));

    eq::fabric::ConfigParams configParams;
	eq::Config* config = server->chooseConfig( configParams );
	TEST( config );
	TEST( config->init( co::uint128_t( )));

	// 3.- Force frame generation
	config->startFrame( co::uint128_t() );
	config->finishFrame();

	// 4.- Make sure EQ is properly shut down
	tearDown( server, client, config );

	// 5.- Verify results
    TEST( verifyFileExists( GENERATED_FILENAME ) );

    // 6.- Remove generated file
    deleteFile( GENERATED_FILENAME );

    return EXIT_SUCCESS;
}

void setupDumpImage()
{
	eq::server::Global * global = eq::server::Global::instance();
	global->setChannelSAttribute(eq::server::Channel::SATTR_DUMP_IMAGE,
								 PREFIX);
	global->setWindowIAttribute(eq::server::Window::IATTR_HINT_DRAWABLE, 0);
}

void deleteFile( const std::string& filename )
{
	remove( filename.c_str() );
}

bool verifyFileExists( const std::string& filename )
{
	std::ifstream ifile(filename);
	return ifile;
}

void tearDown( eq::ServerPtr& server, eq::ClientPtr& client,
			   eq::Config* config)
{
	config->exit();
	server->releaseConfig( config );
	client->disconnectServer( server );
	client->exitLocal();
	eq::exit();
}
