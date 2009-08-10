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

#ifndef EQNBODY_CONFIGEVENT_H
#define EQNBODY_CONFIGEVENT_H

#include <eq/eq.h>

namespace eqNbody
{
	struct ConfigEvent : public eq::ConfigEvent
	{
	public:
		ConfigEvent();
		
		enum Type
		{
			DATA_CHANGED = eq::Event::USER,
			PROXY_CHANGED
		};
				
		unsigned int	_version;
		unsigned int	_proxyID;
		float			_range[2];
	};
	
	std::ostream& operator << ( std::ostream& os, const ConfigEvent* event );
}

#endif // EQNBODY_CONFIGEVENT_H

