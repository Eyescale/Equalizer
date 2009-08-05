/*
 * Copyright (c) 2009, Philippe Robert <probert@eyescale.ch> 
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

#include "client.h"
#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <eq/eq.h>
#include <stdlib.h>

using namespace eq::base;
using namespace std;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config* createConfig( eq::ServerPtr parent )	{ return new eqNbody::Config( parent ); }
    virtual eq::Node* createNode( eq::Config* parent )			{ return new eqNbody::Node( parent ); }
    virtual eq::Pipe* createPipe( eq::Node* parent )			{ return new eqNbody::Pipe( parent ); }
    virtual eq::Window* createWindow( eq::Pipe* parent )		{ return new eqNbody::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )	{ return new eqNbody::Channel( parent ); }
};

int main( const int argc, char** argv )
{
    eqNbody::LocalInitData ld;
    NodeFactory nodeFactory;
	
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }
    
    RefPtr< eqNbody::Client > client = new eqNbody::Client( ld );
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
    }

	// Init
	if( client->init() != EXIT_SUCCESS ) {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
	}

	// Run the simulation
    client->run();

	// Exit
	if( client->exit() != EXIT_SUCCESS ) {
        EQERROR << "Can't exit client" << endl;
	}
	
    client->exitLocal();	
    client = 0;
	
    eq::exit();
    return EXIT_SUCCESS;	
}
