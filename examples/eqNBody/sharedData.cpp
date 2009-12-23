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

#include "sharedData.h"
#include "sharedDataProxy.h"
#include "pipe.h"
#include "config.h"
#include "controller.h"

namespace eqNbody
{
	SharedData::SharedData(Config *cfg) : _cfg(cfg)
	{
		EQASSERT(_cfg);
	}
	
	SharedData::~SharedData()
	{
		if(_cfg)
		{
			_cfg = 0;
		}
	}
			
	void SharedData::registerMemory( const eq::Range& range )
	{		
		// Initialise the local proxy
		unsigned int offset = range.start * _frameData.getNumBodies() * 4;
		unsigned int numBytes = (range.end - range.start) * _frameData.getNumBytes(); 
		
		SharedDataProxy *shMem = new SharedDataProxy();
		_proxies.push_back( shMem );
		
		// Register the proxy object
		_cfg->registerObject( shMem );
		shMem->init( offset, numBytes, _frameData.getPos(), _frameData.getVel(), _frameData.getCol() );		
		uint32_t version = shMem->commit(); 
		
		// Let the app know which range is covered by this proxy
		_sendEvent(ConfigEvent::DATA_CHANGED, version, shMem->getID(), range);		
	}
	
	void SharedData::mapMemory()
	{
		SharedDataProxy *shMem = _proxies[0];
		
		// Initialise the remote shared memory proxies
		for(unsigned int i=0; i< _frameData.getNumDataProxies(); i++) {
			unsigned int pid = _frameData.getProxyID(i);
			
			if( (pid != shMem->getID()) ) {
				SharedDataProxy *readMem = new SharedDataProxy();
				
				readMem->init( _frameData.getPos(), _frameData.getVel(), _frameData.getCol() );
				_proxies.push_back( readMem );
				
				const bool mapped = _cfg->mapObject( readMem, pid );
				EQASSERT( mapped );
			}
		}			
	}
	
	void SharedData::syncMemory()
	{
		for(unsigned int i=1; i< _frameData.getNumDataProxies(); i++) {			
			unsigned int pid = _proxies[i]->getID();
			unsigned int version = _frameData.getVersionForProxyID(pid);
			
			// ...and sync!
			_proxies[i]->sync(version);
		}
	}
	
	void SharedData::updateMemory(const eq::Range& range, Controller *controller)
	{
		SharedDataProxy *local = _proxies[0];

		controller->getArray(BODYSYSTEM_POSITION, *local);
		controller->getArray(BODYSYSTEM_VELOCITY, *local);			
		
		// Commit the local changes
		uint32_t version = local->commit();
		
		// Tell the others what version to sync.
		_sendEvent(ConfigEvent::PROXY_CHANGED, version, local->getID(), range);
	}
	
	void SharedData::_sendEvent(ConfigEvent::Type type, unsigned int version, unsigned int pid, const eq::Range& range)
	{
		ConfigEvent event;

		event.data.type = type;
		event._version = version;
		event._range[0] = range.start;
		event._range[1] = range.end;
		event._proxyID = pid;
		
		_cfg->sendEvent( event );
	}	
}