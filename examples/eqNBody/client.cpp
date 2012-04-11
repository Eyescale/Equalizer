/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "client.h"

#include "config.h"
#include "initData.h"

#include <stdlib.h>

namespace eqNbody
{
    
Client::Client( const InitData& initData )
        : _initData( initData )
        , _config( 0 )
{}
    
int Client::init()
{
    LBASSERT( !_config );

    // 1. connect to server
    _server = new eq::Server;
    if( !connectServer( _server ))
    {
        EQERROR << "Can't open server" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. choose config
    eq::ConfigParams configParams;
    _config = static_cast<Config*>(_server->chooseConfig( configParams ));

    if( !_config )
    {
        EQERROR << "No matching config on server" << std::endl;
        disconnectServer( _server );
        return EXIT_FAILURE;
    }

    // 3. init config
    _config->setInitData( _initData );
    if( !_config->init() )
    {
        _server->releaseConfig( _config );
        disconnectServer( _server );
        return EXIT_FAILURE;
    }
    else if( _config->getError( ))
        EQWARN << "Error during initialization: " << _config->getError()
               << std::endl;

    return EXIT_SUCCESS;
}

int Client::exit()
{
    LBASSERT( _config );

    // Exit config
    _config->exit();
        
    // Cleanup
    _server->releaseConfig( _config );
    if( !disconnectServer( _server )) {
        EQERROR << "Client::disconnectServer failed" << std::endl;
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
}
