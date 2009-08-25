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

#include "frameData.h"
#include "nbody.h"

#include <vector_types.h>
#include <cuda.h>
#include <cuda_runtime_api.h>

#ifdef CUDART_VERSION >= 2020
# define ENABLE_HOSTALLOC
#endif

extern "C" {
	float v3_normalize(float3& vector)
	{
		float dist = sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
		if (dist > 1e-6)
		{
			vector.x /= dist;
			vector.y /= dist;
			vector.z /= dist;
		}
		return dist;
	}
	
	float v3_dot(float3 v0, float3 v1)
	{
		return v0.x*v1.x+v0.y*v1.y+v0.z*v1.z;
	}
	
	float3 v3_cross(float3 v0, float3 v1)
	{
		float3 rt;
		rt.x = v0.y*v1.z-v0.z*v1.y;
		rt.y = v0.z*v1.x-v0.x*v1.z;
		rt.z = v0.x*v1.y-v0.y*v1.x;	
		return rt;
	}
};

namespace eqNbody
{
	
	FrameData::FrameData() : _statistics( true ) , _numDataProxies(0), _hPos(0), _hVel(0), _hCol(0)
	{		
		_numBodies		= 0;
		_deltaTime		= 0.0f;
		_clusterScale	= 0.0f;
        _velocityScale	= 0.0f;
		_newParameters	= false;
	}
	
	FrameData::~FrameData()
	{
		_numDataProxies = 0;
	}
	
	void FrameData::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
	{		
		eq::Object::serialize( os, dirtyBits );
		
		if( dirtyBits & DIRTY_DATA ) {
			if(_hPos && _hVel && _hCol) {
				os.write(_hPos, sizeof(float)*_numBodies*4);
				os.write(_hVel, sizeof(float)*_numBodies*4);
				os.write(_hCol, sizeof(float)*_numBodies*4);
			}
		}
		
		if( dirtyBits & DIRTY_FLAGS ) {
			os << _statistics << _numBodies << _clusterScale << _velocityScale << _deltaTime << _newParameters;
		}		
		
		if( dirtyBits & DIRTY_PROXYDATA ) {
			os << _numDataProxies;
			os.write(&_dataProxyID[0][0], sizeof(unsigned int) * MAX_NGPUS * 2);			
			os.write(&_dataRanges[0], sizeof(float) * MAX_NGPUS);			
		}
	}
	
	void FrameData::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
	{
		eq::Object::deserialize( is, dirtyBits );
		
		if( dirtyBits & DIRTY_DATA ) {
			if(_hPos && _hVel && _hCol) {
				is.read(_hPos, sizeof(float)*_numBodies*4);
				is.read(_hVel, sizeof(float)*_numBodies*4);
				is.read(_hCol, sizeof(float)*_numBodies*4);
			}
		}
		
		if( dirtyBits & DIRTY_FLAGS ) {
			is >> _statistics >> _numBodies >> _clusterScale >> _velocityScale >> _deltaTime >> _newParameters;
		}
		
		if( dirtyBits & DIRTY_PROXYDATA ) {
			is >> _numDataProxies;
			is.read(&_dataProxyID[0][0], sizeof(unsigned int) * MAX_NGPUS * 2);
			is.read(&_dataRanges[0], sizeof(float) * MAX_NGPUS);
		}
	}
	
	void FrameData::addProxyID( unsigned int pid, const float *range )
	{
		EQASSERT(_numDataProxies < MAX_NGPUS);
		
		_dataProxyID[_numDataProxies][0] = pid;  
		_dataProxyID[_numDataProxies][1] = 0;

		_dataRanges[_numDataProxies++] = (range[1] - range[0]);
		setDirty( DIRTY_PROXYDATA );
	}
	
	void FrameData::updateProxyID( unsigned int pid, unsigned int version, const float *range )
	{
		// TODO: Better use a hash here!
		for(unsigned int i=0; i< _numDataProxies; i++) {
			if( pid == _dataProxyID[i][0]) {
				_dataProxyID[i][1] = version;
				_dataRanges[i] = (range[1] - range[0]);
				break;
			}
		}
		setDirty( DIRTY_PROXYDATA );
	}
	
	uint32_t FrameData::commit()
	{
		bool ret = eq::Object::commit();
		EQASSERT(ret);
		
		for(unsigned int i=0; i< _numDataProxies; i++) {
			_dataRanges[i] = 0.0f;
		}		
		
		return ret;
	}
	
	bool FrameData::isReady()
	{
		float length = 0.0f;
		
		for(unsigned int i=0; i<_numDataProxies; i++) {
			length += _dataRanges[i];
		}
		
		// Return true if range [0 1] is covered, otherwise false
		return (length == 1.0f) ? true : false;
	}
	
	unsigned int FrameData::getVersionForProxyID( unsigned int pid ) const
	{		
		int version = -1;
		
		// TODO: Better use a hash here!
		for(unsigned int i=0; i< _numDataProxies; i++) {
			if( pid == getProxyID(i) ) {
				version = getProxyVersion(i); 
				break;
			}
		}
		
		EQASSERT(version != -1)
		
		return (unsigned int)version;
	}
	
	void FrameData::toggleStatistics()
	{
		_statistics = !_statistics;
		setDirty( DIRTY_FLAGS );
	}
	
	void FrameData::init(const unsigned int numBodies)
	{
		_numBodies	= numBodies;
				
		setDirty( DIRTY_FLAGS );
	}
	
	void FrameData::initHostData()
	{		
#ifdef ENABLE_HOSTALLOC
        allocateHostArrays(&_hPos, &_hVel, &_hCol, _numBodies*4*sizeof(float));
#else
		_hPos		= new float[_numBodies*4];
		_hVel		= new float[_numBodies*4];
		_hCol		= new float[_numBodies*4];
#endif
		
		memset(_hPos, 0, _numBodies*4*sizeof(float));
		memset(_hVel, 0, _numBodies*4*sizeof(float));
		memset(_hCol, 0, _numBodies*4*sizeof(float));
		
		setDirty( DIRTY_DATA );
	}
	
	void FrameData::exit()
	{
		_numDataProxies = 0;
		_numBodies		= 0;

#ifdef ENABLE_HOSTALLOC
        deleteHostArrays(_hPos, _hVel, _hCol);
#else
		delete [] _hPos;
		delete [] _hVel;	
		delete [] _hCol;	
#endif
	}
	
	void FrameData::updateParameters(NBodyConfig config, float clusterScale, float velocityScale, float ts)
	{		
		_clusterScale	= clusterScale;
        _velocityScale	= velocityScale;
		_deltaTime		= ts;
		_newParameters	= true;
		
		_randomizeData(config);
		
		setDirty( DIRTY_DATA );
		setDirty( DIRTY_FLAGS );
	}
	
	void FrameData::_randomizeData(NBodyConfig config)
	{
		switch(config)
		{
			default:
			case NBODY_CONFIG_RANDOM:
			{
				float scale	 = _clusterScale * std::max(1.0f, _numBodies / (1024.f));
				float vscale = _velocityScale * scale;
				
				int p = 0, v = 0;
				unsigned int i = 0;
				while (i < _numBodies) 
				{
					float3 point;
					//const int scale = 16;
					point.x = rand() / (float) RAND_MAX * 2 - 1;
					point.y = rand() / (float) RAND_MAX * 2 - 1;
					point.z = rand() / (float) RAND_MAX * 2 - 1;
					float lenSqr = v3_dot(point, point);
					if (lenSqr > 1)
						continue;
					float3 velocity;
					velocity.x = rand() / (float) RAND_MAX * 2 - 1;
					velocity.y = rand() / (float) RAND_MAX * 2 - 1;
					velocity.z = rand() / (float) RAND_MAX * 2 - 1;
					lenSqr = v3_dot(velocity, velocity);
					if (lenSqr > 1)
						continue;
					
					_hPos[p++] = point.x * scale; // pos.x
					_hPos[p++] = point.y * scale; // pos.y
					_hPos[p++] = point.z * scale; // pos.z
					_hPos[p++] = 1.0f; // mass
					
					_hVel[v++] = velocity.x * vscale; // pos.x
					_hVel[v++] = velocity.y * vscale; // pos.x
					_hVel[v++] = velocity.z * vscale; // pos.x
					_hVel[v++] = 1.0f; // inverse mass
					
					i++;
				}
			}
				break;
			case NBODY_CONFIG_SHELL:
			{
				float scale = _clusterScale;
				float vscale = scale * _velocityScale;
				float inner = 2.5f * scale;
				float outer = 4.0f * scale;
				
				int p = 0, v=0;
				unsigned int i = 0;
				while (i < _numBodies)//for(int i=0; i < numBodies; i++) 
				{
					float x, y, z;
					x = rand() / (float) RAND_MAX * 2 - 1;
					y = rand() / (float) RAND_MAX * 2 - 1;
					z = rand() / (float) RAND_MAX * 2 - 1;
					
					float3 point = {x, y, z};
					float len = v3_normalize(point);
					if (len > 1)
						continue;
					
					_hPos[p++] =  point.x * (inner + (outer - inner) * rand() / (float) RAND_MAX);
					_hPos[p++] =  point.y * (inner + (outer - inner) * rand() / (float) RAND_MAX);
					_hPos[p++] =  point.z * (inner + (outer - inner) * rand() / (float) RAND_MAX);
					_hPos[p++] = 1.0f;
					
					x = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
					y = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
					z = 1.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
					float3 axis = {x, y, z};
					v3_normalize(axis);
					
					if (1 - v3_dot(point, axis) < 1e-6)
					{
						axis.x = point.y;
						axis.y = point.x;
						v3_normalize(axis);
					}
					//if (point.y < 0) axis = scalevec(axis, -1);
					float3 vv = {_hPos[4*i], _hPos[4*i+1], _hPos[4*i+2]};
					vv = v3_cross(vv, axis);
					_hVel[v++] = vv.x * vscale;
					_hVel[v++] = vv.y * vscale;
					_hVel[v++] = vv.z * vscale;
					_hVel[v++] = 1.0f;
					
					i++;
				}
			}
				break;
			case NBODY_CONFIG_EXPAND:
			{
				float scale = _clusterScale * std::max(1.0f, _numBodies / (1024.f));
				float vscale = scale * _velocityScale;
				
				int p = 0, v = 0;
				for(unsigned int i=0; i < _numBodies;) 
				{
					float3 point;
					
					point.x = rand() / (float) RAND_MAX * 2 - 1;
					point.y = rand() / (float) RAND_MAX * 2 - 1;
					point.z = rand() / (float) RAND_MAX * 2 - 1;
					
					float lenSqr = v3_dot(point, point);
					if (lenSqr > 1)
						continue;
					
					_hPos[p++] = point.x * scale; // pos.x
					_hPos[p++] = point.y * scale; // pos.y
					_hPos[p++] = point.z * scale; // pos.z
					_hPos[p++] = 1.0f; // mass
					
					_hVel[v++] = point.x * vscale; // pos.x
					_hVel[v++] = point.y * vscale; // pos.x
					_hVel[v++] = point.z * vscale; // pos.x
					_hVel[v++] = 1.0f; // inverse mass
					
					i++;
				}
			}
				break;
		}
		
		if (_hCol)
		{
			int v = 0;
			for(unsigned int i=0; i < _numBodies; i++) 
			{
				_hCol[v++] = rand() / (float) RAND_MAX;
				_hCol[v++] = rand() / (float) RAND_MAX;
				_hCol[v++] = rand() / (float) RAND_MAX;
				_hCol[v++] = 1.0f;
			}
		}		
	}
}

