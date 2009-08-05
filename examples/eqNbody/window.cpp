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
}
