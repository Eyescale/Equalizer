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

#include "initData.h"

#include "client.h"
#include "frameData.h"

using namespace eq::base;
using namespace std;

namespace eqNbody
{
	
	InitData::InitData() : _frameDataID( EQ_ID_INVALID )
	{
		_damping	= 0.995f;
		_p			= 256;
		_q			= 1;
		_numBodies	= NUM_BODIES;
	}
	
	InitData::~InitData()
	{
		setFrameDataID( EQ_ID_INVALID );
	}
	
	void InitData::getInstanceData( eq::net::DataOStream& os )
	{
		os << _frameDataID;
	}
	
	void InitData::applyInstanceData( eq::net::DataIStream& is )
	{
		is >> _frameDataID;
		EQASSERT( _frameDataID != EQ_ID_INVALID );
	}
}
