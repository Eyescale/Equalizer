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

#include "pipe.h"

#include "config.h"
#include "frameData.h"

using namespace eq::base;
using namespace std;

namespace eqNbody
{			
	Pipe::Pipe( eq::Node* parent ) : eq::Pipe( parent ) 
	{
	}
		
	bool Pipe::configInit( const uint32_t initID )
	{
		if( !eq::Pipe::configInit( initID )) {
			return false;
		}
		
		Config*         config      = static_cast<Config*>( getConfig() );
		const InitData& initData    = config->getInitData();
		const uint32_t  frameDataID = initData.getFrameDataID();

		_data = new SharedData(config);
		EQASSERT( _data );

		FrameData& fd = const_cast<FrameData&>(_data->getFrameData());
		fd.init(initData.getNumBodies());

		const bool mapped = config->mapObject( &fd, frameDataID );
		EQASSERT( mapped );

		return mapped;
	}
	
	bool Pipe::configExit()
	{
		eq::Config* config = getConfig();
		FrameData& fd = const_cast<FrameData&>(_data->getFrameData());

		config->unmapObject( &fd );
		
		EQASSERT( _data );
		delete _data;
						
		return eq::Pipe::configExit();
	}
		
	void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
	{								
		static bool isInitialized = false;
		FrameData& fd = const_cast<FrameData&>(_data->getFrameData());

		// Allocate the CUDA memory after proper CUDA initialisation!
		if(isInitialized == false) {
			fd.initHostData();			
			isInitialized = true;
		}		
		
		// Sync for the next frame - wait for the data broadcast!
		fd.sync( frameID );
		eq::Pipe::frameStart( frameID, frameNumber );
	}
					
}
