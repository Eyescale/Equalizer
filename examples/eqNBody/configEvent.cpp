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

#include "configEvent.h"

using namespace std;

namespace eqNbody
{	
	ConfigEvent::ConfigEvent() : _proxyID(0)
	{
		_range[0] = _range[1] = 0.0f;
		size = sizeof( ConfigEvent );
	}
	
	std::ostream& operator << ( std::ostream& os, const ConfigEvent* event )
	{
		switch( event->data.type )
		{
			case ConfigEvent::DATA_CHANGED:
				os << "datachanged";
				break;

			case ConfigEvent::PROXY_CHANGED:
				os << "proxychanged";
				break;
				
			default:
				os << static_cast< const eq::ConfigEvent* >( event );
				return os;
		}
						
		return os;
	}
	
}
