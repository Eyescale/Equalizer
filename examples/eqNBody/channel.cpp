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

#include "channel.h"

#include "client.h"
#include "initData.h"
#include "config.h"
#include "pipe.h"

#include "controller.h"
#include "sharedData.h"

using namespace eq::base;
using namespace std;

namespace eqNbody
{
	Channel::Channel( eq::Window* parent ) : eq::Channel( parent ) ,
		_registerMem(true),
		_mapMem(true)
	{
		_controller = new Controller();
	}

	Channel::~Channel()
	{
		if (_controller) {
			delete _controller;
			_controller = 0;
		}		
	}
	
	bool Channel::configInit( const uint32_t initID )
	{
		if( !eq::Channel::configInit( initID )) {
			return false;
		}
		
		// Initialize the CUDA controller
		const InitData& id = static_cast<Config*>( getConfig() )->getInitData();
		SharedData& sd = static_cast<Pipe*>( getPipe() )->getSharedData();

		bool sysready = _controller->init(id, sd.getPos(), true);
		EQASSERT( sysready );		
		
		return true;
	}
		
	void Channel::frameDraw( const uint32_t frameID )
	{						
		const eq::Range& range = getRange();
		SharedData& sd = static_cast<Pipe*>( getPipe() )->getSharedData();

		// 1st, register the local memory
		if( _registerMem ) {
			sd.registerMemory( getRange() );
			_registerMem = false;

			// Make sure all proxies are mapped before cont'ing
			return; 
		}

		// 2nd, map remote memory
		if( _mapMem ) {
			sd.mapMemory();
			_mapMem = false;			
		}
		
		// 3rd, synchronize the shared memory
		sd.syncMemory();
		
		// 4th, update the GPU memory and run one simulation step
		_controller->setArray(BODYSYSTEM_POSITION, sd.getPos(), sd.getNumBytes());
		_controller->setArray(BODYSYSTEM_VELOCITY, sd.getVel(), sd.getNumBytes());
		_controller->compute(frameID, sd.getTimeStep(), range);				

		// 5th, draw the stars
		eq::Channel::frameDraw( frameID );
		_controller->draw(sd.getPos(), sd.getCol());

#ifndef NDEBUG
		outlineViewport();
#endif
		
		// Finally, redistribute the newly computed data from the GPU to all
		// interested mappers
		sd.updateMemory(range, _controller);
	}			
}
