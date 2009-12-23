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

#include "config.h"
#include <eq/client/types.h>

using namespace eq;
using namespace eq::base;
using namespace std;

namespace eqNbody
{
	
	Config::Config( eq::base::RefPtr< eq::Server > parent )
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
		deregisterObject( &_initData );
		deregisterObject( &_frameData );
		
		_initData.setFrameDataID( EQ_ID_INVALID );
	}
	
	
	void Config::mapData( const uint32_t initDataID )
	{		
		if( _initData.getID() == EQ_ID_INVALID )
		{
			EQCHECK( mapObject( &_initData, initDataID ));
			unmapObject( &_initData ); // data was retrieved, unmap immediately
		}
		else  // appNode, _initData is registered already
		{
			EQASSERT( _initData.getID() == initDataID );
		}
	}
	
	void Config::unmapData()
	{
		unmapObject( &_initData );
	}
	
	uint32_t Config::startFrame()
	{
		static bool isInitialized = false;
		
		// Allocate the CUDA memory after the CUDA device initialisation!
		if(isInitialized == false) {
			_frameData.initHostData();
			_frameData.updateParameters(NBODY_CONFIG_SHELL, 2.12f, 2.98f, 0.016f);
			
			isInitialized = true;
		}		
		
		// Get current version...
		uint32_t version = _frameData.getVersion();
				
		_redraw = false;
		return eq::Config::startFrame( version );
	}
	
	bool Config::needsRedraw()
	{
		return ( _redraw );
	}
	
	bool Config::handleEvent( const eq::ConfigEvent* event )
	{				
		switch( event->data.type )
		{
			case ConfigEvent::DATA_CHANGED:
				_registerData(static_cast< const ConfigEvent* >( event ));
				if( _readyToCommit() ) {
					_frameData.commit();		// broadcast changed data to all clients
				}
				break;

			case ConfigEvent::PROXY_CHANGED:
				{
					_updateData(static_cast< const ConfigEvent* >( event ));
					if( _readyToCommit() ) {
						_updateSimulation();	// update the simulation every nth frame
						_frameData.commit();	// broadcast changed data to all clients
					}
				}
				break;
				
			case eq::Event::KEY_PRESS:
				if( _handleKeyEvent( event->data.keyPress ))
				{
					_redraw = true;
					return true;
				}
				break;
								
			case eq::Event::WINDOW_EXPOSE:
			case eq::Event::WINDOW_RESIZE:
			case eq::Event::WINDOW_CLOSE:
			case eq::Event::VIEW_RESIZE:
				_redraw = true;
				break;
				
			default:
				break;
		}
		
		_redraw |= eq::Config::handleEvent( event );
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
		static int ctr = 0;		// frame counter
		static int demo = 0;	// demo config
		
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
	
	void Config::_registerData(const ConfigEvent* event)
	{	
		_frameData.addProxyID(event->_proxyID, event->_range);
	}	

	void Config::_updateData(const ConfigEvent* event)
	{	
		_frameData.updateProxyID(event->_proxyID, event->_version, event->_range);
	}	
}
