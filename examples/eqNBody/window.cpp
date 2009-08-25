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

#include "window.h"

#include "pipe.h"
#include "config.h"
#include "configEvent.h"
#include "frameData.h"

#include <fstream>
#include <sstream>

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>

using namespace std;

namespace eqNbody
{		
	bool Window::configInit( const uint32_t initID )
	{
		// Determine window mode (and offset)
		const string& name = getName();
		size_t sepos = name.find(":");

		EQASSERT(sepos != string::npos);

		std::string tname = name.substr(0,sepos);

		if(tname == "CUDA") {
			_mode = WINDOW_CUDA;
		}
		else if(tname == "CUDA_GL") {
			_mode = WINDOW_CUDA_GL;
		}
		else {
			EQWARN << "Unhandled Window type '" << tname << "'" << std::endl;
			return false;
		}
		
		// Determine CUDA device
		Pipe* pipe = static_cast<Pipe*>(getPipe());
		_device = pipe->getDevice();
		
		if( !eq::Window::configInit( initID )) {
			return false;
		}
		
		return configInitCUDA();
	}
				
	bool Window::configInitCUDA()
	{
		cudaDeviceProp	_props;

		// Setup the CUDA device
		if( _device == -1 ) {
			_device = _getMaxGflopsDeviceId();
			EQWARN << "No CUDA device, using the fastest device: " << _device << std::endl;
		}
		
		if( _mode == WINDOW_CUDA_GL ) {
			cudaGLSetGLDevice( _device );
		}
		else if( _mode == WINDOW_CUDA ) {
			cudaSetDevice( _device );
		}
		else {
			return false;
		}
		
		cudaGetDevice(&_device);
		cudaGetDeviceProperties(&_props, _device);
		
		cudaError_t err = cudaGetLastError();
		if( cudaSuccess != err) 
		{
			EQERROR << "CUDA initialization error: " << cudaGetErrorString( err) << std::endl;
			exit(EXIT_FAILURE);
		}                         
		
		EQINFO << "Using CUDA device: " << _device << std::endl;
		
		return true;
	}

	void Window::swapBuffers()
	{
		const Pipe*              pipe      = static_cast<Pipe*>( getPipe( ));
		const FrameData&         frameData = pipe->getFrameData();
		const eq::ChannelVector& channels  = getChannels();
		
		if( frameData.useStatistics() && !channels.empty( ))
			EQ_GL_CALL( channels.back()->drawStatistics( ));
		
		eq::Window::swapBuffers();
	}		
	
	int Window::_getMaxGflopsDeviceId()
	{		
#if __DEVICE_EMULATION__
		return 0;
#else		
        int device_count = 0;
		cudaGetDeviceCount( &device_count );
        
		cudaDeviceProp device_properties;
		int max_gflops_device = 0;
		int max_gflops = 0;
		
		int current_device = 0;
		cudaGetDeviceProperties( &device_properties, current_device );
		max_gflops = device_properties.multiProcessorCount * device_properties.clockRate;
		++current_device;
		
		while( current_device < device_count )
		{
			cudaGetDeviceProperties( &device_properties, current_device );
			int gflops = device_properties.multiProcessorCount * device_properties.clockRate;
			if( gflops > max_gflops )
			{
				max_gflops        = gflops;
				max_gflops_device = current_device;
			}
			++current_device;
		}
		return max_gflops_device;
#endif
	}
	
}
