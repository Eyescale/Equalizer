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

#include "channel.h"

#include "client.h"
#include "frameData.h"
#include "initData.h"
#include "config.h"
#include "pipe.h"
#include "window.h"

using namespace eq::base;
using namespace std;

namespace eqNbody
{
	void Channel::frameClear( const uint32_t frameID )
	{
		EQ_GL_CALL( applyBuffer( ));
		EQ_GL_CALL( applyViewport( ));
		
		glClearColor( 0.f, 0.f, 0.f, 1.0f );
		EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
	}
	
	void Channel::frameDraw( const uint32_t frameID )
	{						
		const Pipe* pipe = static_cast<const Pipe*>( getPipe( ));
		const FrameData& fd = pipe->getFrameData();

		// 1st, initialise the local proxy object
		static bool isInitialised1 = false;
		if(isInitialised1 == false) {
			_initLocalProxy();
			isInitialised1 = true;
			return;
		}

		// 2nd, initialise the CUDA part
		static bool isInitialised2 = false;
		if(isInitialised2 == false) {
			_initCUDAController();
			isInitialised2 = true;
		}
		
		_compute(frameID, fd);
		_draw(frameID, fd);
		_assemble(frameID, fd);
	}	

	void Channel::_compute(const uint32_t frameID, const FrameData& fd)
	{
		const eq::Range& range = getRange();

		// Sync all remote data, if needed. 
		// In this example there will always be new data to be synchronized.
		_syncDataProxies(fd);
		
		// Upload the current data from the host to the GPU
		_controller.setArray(BODYSYSTEM_POSITION, fd);
		_controller.setArray(BODYSYSTEM_VELOCITY, fd);
		
		// Compute the next step in the simulation
		_controller.compute(frameID, fd, range);				
	}

	void Channel::_draw(const uint32_t frameID, const FrameData& fd)
	{
		// Setup OpenGL state and draw
		eq::Channel::frameDraw( frameID );
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glTranslatef( 0.0f, 0.0f, -50.0f );
		
		_controller.draw(fd);
		
#ifndef NDEBUG
		outlineViewport();
#endif
	}

	void Channel::_assemble(const uint32_t frameID, const FrameData& fd)
	{
		const eq::Range& range = getRange();

		// Transfer the modified data from the device to the host.
		// In this example there will always be new data!
		_controller.getArray(BODYSYSTEM_POSITION, _dataProxy[0]);
		_controller.getArray(BODYSYSTEM_VELOCITY, _dataProxy[0]);			
		
		// Commit the local changes
		uint32_t version = _dataProxy[0].commit();

		// Tell the others what version to sync.
		_sendEvent(ConfigEvent::PROXY_CHANGED, version, _dataProxy[0].getID(), range);
	}
	
	void Channel::_initLocalProxy()
	{		
		Config* config		= static_cast<Config*>(getConfig());
		const Pipe* pipe	= static_cast<Pipe*>( getPipe( ));
		const FrameData& fd	= pipe->getFrameData();
		
		const eq::Range& range = getRange();
		
		// Initialise the local proxy
		unsigned int offset = range.start * fd.getNumBodies() * 4;
		unsigned int numBytes = (range.end - range.start) * fd.getNumBytes(); 
		
		// Register the proxy object
		config->registerObject( &_dataProxy[0] );

		// Init the local data proxy
		_dataProxy[0].init(offset, numBytes, fd.getPos(), fd.getVel(), fd.getCol());		
		uint32_t version = _dataProxy[0].commit(); 
		
		// Let the app know which range is covered by this proxy
		_sendEvent(ConfigEvent::DATA_CHANGED, version, _dataProxy[0].getID(), range);
	}
	
	void Channel::_initCUDAController()
	{
		Config* config		= static_cast<Config*>(getConfig());
		const Pipe* pipe	= static_cast<Pipe*>( getPipe( ));
		const FrameData& fd	= pipe->getFrameData();
		
		Window *w = static_cast<eqNbody::Window*>(getWindow());			
		const InitData& initData = config->getInitData();		
				
		unsigned int mode = w->getMode();
		bool usePBO	= (mode == WINDOW_CUDA_GL) ? true : false;
						
		switch(mode) {
			case WINDOW_CUDA_GL:
			case WINDOW_CUDA:
			{
				bool sysready = _controller.init(initData, fd.getPos(), usePBO);
				EQASSERT( sysready );
				break;
			}
			case WINDOW_GL:
				EQINFO << "GL window - no CUDA controller initialisation!" << std::endl;
				break;
			default:
				EQWARN << "Unknown mode - no CUDA controller initialisation!" << std::endl;
				break;
		}
		
		_initDataProxies(fd);
	}
			
	void Channel::_initDataProxies(const FrameData& frameData)
	{
		Config* config		= static_cast<Config*>(getConfig());
		
		for(unsigned int i=0, j=1; i< frameData.getNumDataProxies(); i++) {
			unsigned int id = frameData.getProxyID(i);
			
			if( (id != _dataProxy[0].getID())) {
				_dataProxy[j].init(frameData.getPos(), frameData.getVel(), frameData.getCol());
				
				const bool mapped = config->mapObject( &_dataProxy[j++], id );
				EQASSERT( mapped );
			}
		}			
	}
	
	void Channel::_syncDataProxies(const FrameData& frameData)
	{
		for(unsigned int i=1; i< frameData.getNumDataProxies(); i++) {			
			unsigned int pid = _dataProxy[i].getID();
			unsigned int version = frameData.getVersionForProxyID(pid);

			// ...and sync!
			_dataProxy[i].sync(version);
		}
	}
	
	void Channel::_sendEvent(ConfigEvent::Type type, unsigned int version, unsigned int pid, const eq::Range& range)
	{
		Config* config		= static_cast<Config*>(getConfig());
		const string& name	= getName();
		
		ConfigEvent event;
		
		if( name.empty( )) {    
			snprintf( event.data.user.data, 32, "%p", this );
		}
		else {
			snprintf( event.data.user.data, 32, "%s", name.c_str( ));
		}
		
		event.data.user.data[31] = '\0';
		
		event.data.type = type;
		event._version = version;
		event._range[0] = range.start;
		event._range[1] = range.end;
		event._proxyID = pid;
		
		config->sendEvent( event );
	}
}
