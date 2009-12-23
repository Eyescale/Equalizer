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

#include "node.h"

#include "client.h"
#include "initData.h"
#include "config.h"

using namespace eq::base;
using namespace std;

namespace eqNbody
{
	bool Node::configInit( const uint32_t initID )
	{
		if( !eq::Node::configInit( initID ))
			return false;
		
		// All render data is static or multi-buffered, we can run asynchronously
		if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED ) {
			setIAttribute( IATTR_THREAD_MODEL, eq::ASYNC );
		}
		
		Config* config = static_cast< Config* >( getConfig( ));
		config->mapData( initID );

		return true;
	}
	
	bool Node::configExit()
	{
		Config* config = static_cast< Config* >( getConfig( ));
		config->unmapData();
		
		return eq::Node::configExit();
	}
	
}
