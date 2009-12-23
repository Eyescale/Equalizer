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

#include "window.h"

#include "pipe.h"
#include "frameData.h"

using namespace std;

namespace eqNbody
{						
	void Window::swapBuffers()
	{
		Pipe* pipe = static_cast<Pipe*>( getPipe() );
		const SharedData& sharedData = pipe->getSharedData();
		const eq::ChannelVector& channels = getChannels();
		
		if( sharedData.useStatistics() && !channels.empty( ))
			EQ_GL_CALL( channels.back()->drawStatistics( ));
		
		eq::Window::swapBuffers();
	}		
}
