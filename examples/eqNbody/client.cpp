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

#include "config.h"
#include "localInitData.h"

#include <stdlib.h>

using namespace std;

namespace eqNbody
{
	
	Client::Client( const LocalInitData& initData ) : _initData( initData )
	{
		config = NULL;
	}
	
	int Client::init()
	{
		EQASSERT(config == NULL);
		
		// 1. connect to server
		server = new eq::Server;
		if( !connectServer( server ))
		{
			EQERROR << "Can't open server" << endl;
			return EXIT_FAILURE;
		}
		
		// 2. choose config
		eq::ConfigParams configParams;
		config = static_cast<Config*>(server->chooseConfig( configParams ));
		
		if( !config )
		{
			EQERROR << "No matching config on server" << endl;
			disconnectServer( server );
			return EXIT_FAILURE;
		}
		
		// 3. init config
		config->setInitData( _initData );
		if( !config->init() )
		{
			EQERROR << "Config initialization failed: " << config->getErrorMessage() << endl;
			server->releaseConfig( config );
			disconnectServer( server );
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	
	int Client::exit()
	{
		EQASSERT(config != NULL);

		// Exit config
		config->exit();
		
		// Cleanup
		server->releaseConfig( config );
		if( !disconnectServer( server )) {
			EQERROR << "Client::disconnectServer failed" << endl;
			return EXIT_FAILURE;
		}
		
		server = 0;
		return EXIT_SUCCESS;
	}

	void Client::run()
	{		
		// Run main loop
		while( config->isRunning( ) )
		{
			config->startFrame();
			config->finishFrame();
			
			if( !config->needsRedraw()) {
				config->finishAllFrames();
			}
			
			config->handleEvents(); // process all pending events
		}				
	}
	
	bool Client::clientLoop()
	{
		while( true ) // TODO: implement SIGHUP handler to exit?
		{
			if( !eq::Client::clientLoop( ))
				return false;
			EQINFO << "One configuration run successfully executed" << endl;
		}
		return true;
	}
	
}
