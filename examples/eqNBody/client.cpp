/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
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
#include "initData.h"

#include <stdlib.h>

using namespace std;

namespace eqNbody
{
	
	Client::Client( const InitData& initData ) : _initData( initData ), _config(NULL)
	{
	}
	
	int Client::init()
	{
		EQASSERT(_config == NULL);
		
		// 1. connect to server
		_server = new eq::Server;
		if( !connectServer( _server ))
		{
			EQERROR << "Can't open server" << endl;
			return EXIT_FAILURE;
		}
		
		// 2. choose config
		eq::ConfigParams configParams;
		_config = static_cast<Config*>(_server->chooseConfig( configParams ));
		
		if( !_config )
		{
			EQERROR << "No matching config on server" << endl;
			disconnectServer( _server );
			return EXIT_FAILURE;
		}
		
		// 3. init config
		_config->setInitData( _initData );
		if( !_config->init() )
		{
			EQERROR << "Config initialization failed: " << _config->getErrorMessage() << endl;
			_server->releaseConfig( _config );
			disconnectServer( _server );
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
	
	int Client::exit()
	{
		EQASSERT(_config != NULL);

		// Exit config
		_config->exit();
		
		// Cleanup
		_server->releaseConfig( _config );
		if( !disconnectServer( _server )) {
			EQERROR << "Client::disconnectServer failed" << endl;
			return EXIT_FAILURE;
		}
		
		_server = 0;
		return EXIT_SUCCESS;
	}

	void Client::run()
	{		
		// Run main loop
		while( _config->isRunning( ) )
		{
			_config->startFrame();
			_config->finishFrame();
			
			if( !_config->needsRedraw()) {
				_config->finishAllFrames();
			}
			
			_config->handleEvents(); // process all pending events
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
