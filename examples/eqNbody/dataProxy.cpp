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

#include "dataProxy.h"
#include "client.h"

namespace eqNbody
{
	
	DataProxy::DataProxy() : _offset(0), _numBytes(0)
	{			
		_hPos = NULL;
		_hVel = NULL;
		_hCol = NULL;		
	}
		
	void DataProxy::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
	{
		eq::Object::serialize( os, dirtyBits );
		
		if( dirtyBits & DIRTY_DATA ) {
			os << _offset << _numBytes;

			EQASSERT(_hPos != NULL);			
			EQASSERT(_hVel != NULL);

			os.write(_hPos+_offset, _numBytes);
			os.write(_hVel+_offset, _numBytes);
			//os.write(_hCol+_offset, _numBytes);			
		}		
	}
	
	void DataProxy::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
	{
		eq::Object::deserialize( is, dirtyBits );

		if( dirtyBits & DIRTY_DATA ) {
			is >> _offset >> _numBytes;

			EQASSERT(_hPos != NULL);
			EQASSERT(_hVel != NULL);

			is.read(_hPos+_offset, _numBytes);
			is.read(_hVel+_offset, _numBytes);
			//is.read(_hCol+_offset, _numBytes);
		}		
	}
		
	void DataProxy::init(const unsigned int offset, const unsigned int numBytes, float *pos, float *vel, float *col)
	{
		_offset		= offset;
		_numBytes	= numBytes;

		_hPos		= pos;
		_hVel		= vel;
		_hCol		= col;
		
		setDirty( DIRTY_DATA );
	}

	void DataProxy::init(float *pos, float *vel, float *col)
	{
		_hPos		= pos;
		_hVel		= vel;
		_hCol		= col;
		
		setDirty( DIRTY_DATA );
	}
	
	void DataProxy::exit()
	{
		_offset		= 0;
		_numBytes	= 0;
		
		_hPos		= NULL;
		_hVel		= NULL;
		_hCol		= NULL;				
	}
	
	void DataProxy::markDirty()
	{
		setDirty( DIRTY_DATA );
	}
	
}

