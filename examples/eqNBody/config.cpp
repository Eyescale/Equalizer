
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "config.h"

namespace eqNbody
{

Config::Config( eq::ServerPtr parent )
        : eq::Config( parent )
        , _redraw( true )
{
}

bool Config::init()
{
    // init distributed objects
    _frameData.init( _initData.getNumBodies() );
    registerObject( &_frameData );

    _initData.setFrameDataID( _frameData.getID( ));
    registerObject( &_initData );

    // init config
    if( !eq::Config::init( _initData.getID( )))
    {
        _deregisterData();
        return false;
    }

    return true;
}

bool Config::exit()
{
    const bool ret = eq::Config::exit();
    _deregisterData();

    return ret;
}

void Config::_deregisterData()
{
    releaseData();
    deregisterObject( &_frameData );

    _initData.setFrameDataID( 0 );
}


void Config::mapData( const eq::uint128_t& initDataID )
{
    if( !_initData.isAttached( ))
    {
        LBCHECK( mapObject( &_initData, initDataID ));
        releaseData(); // data was retrieved, unmap immediately
    }
    else  // appNode, _initData is registered already
    {
        LBASSERT( _initData.getID() == initDataID );
    }
}

void Config::releaseData()
{
    releaseObject( &_initData );
}

uint32_t Config::startFrame()
{
    static bool isInitialized = false;

    // Allocate the CUDA memory after the CUDA device initialisation!
    if( isInitialized == false ) {
        _frameData.initHostData();
        _frameData.updateParameters( NBODY_CONFIG_SHELL,
                                     2.12f, 2.98f, 0.016f );
        isInitialized = true;
    }

    const eq::uint128_t& version = _frameData.commit();

    _redraw = false;
    return eq::Config::startFrame( version );
}

bool Config::needsRedraw()
{
    return ( _redraw );
}

bool Config::handleEvent( eq::EventICommand command )
{
    switch( command.getEventType( ))
    {
        case DATA_CHANGED:
        {
            _registerData( command );
            if( _readyToCommit() )
                _frameData.commit();    // broadcast changed data to all clients
            return false;
        }

        case PROXY_CHANGED:
        {
            _updateData( command );
            if( _readyToCommit() )
            {
                _updateSimulation();    // update the simulation every nth frame
                _frameData.commit();    // broadcast changed data to all clients
            }
            return false;
        }

        case eq::Event::KEY_PRESS:
        {
            const eq::Event& event = command.get< eq::Event >();
            if( _handleKeyEvent( event.keyPress ))
            {
                _redraw = true;
                return true;
            }
            break;
        }

        case eq::Event::WINDOW_EXPOSE:
        case eq::Event::WINDOW_RESIZE:
        case eq::Event::WINDOW_CLOSE:
        case eq::Event::VIEW_RESIZE:
            _redraw = true;
            break;

        default:
            break;
    }

    _redraw |= eq::Config::handleEvent( command );
    return _redraw;
}

bool Config::_handleKeyEvent( const eq::KeyEvent& event )
{
    switch( event.key )
    {
      case ' ':
          //_frameData.reset();
          return true;

      case 's':
      case 'S':
          _frameData.toggleStatistics();
          return true;

      default:
          return false;
    }
}

bool Config::_readyToCommit()
{
    return _frameData.isReady();
}

void Config::_updateSimulation()
{
    static int ctr = 0;     // frame counter
    static int demo = 0;    // demo config

    ctr++;

    if(ctr > 200) {
        ctr = 0;
        switch(demo) {
          case 0:
              _frameData.updateParameters(NBODY_CONFIG_SHELL, 2.12f, 2.98f, 0.016f);
              demo++;
              break;
          case 1:
              _frameData.updateParameters(NBODY_CONFIG_EXPAND, 0.68f, 20.0f, 0.016f);
              demo++;
              break;
          case 2:
              _frameData.updateParameters(NBODY_CONFIG_RANDOM, 0.16f, 10.0f, 0.016f);
              demo=0;
              break;
        }
    }
}

void Config::_registerData( eq::EventICommand& command )
{
    const eq::uint128_t pid = command.get< eq::uint128_t >();
    const eq::Range range = command.get< eq::Range >();

    _frameData.addProxyID( pid, range );
}

void Config::_updateData( eq::EventICommand& command )
{
    const eq::uint128_t pid = command.get< eq::uint128_t >();
    const eq::Range range = command.get< eq::Range >();
    const eq::uint128_t version = command.get< eq::uint128_t >();

    _frameData.updateProxyID( pid, version, range );
}
}
