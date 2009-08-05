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

#include "controller.h"
#include "initData.h"

#include <GL/glew.h>

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>

#include <eq/base/log.h>

using namespace std;

extern "C"
{
    void cudaInit(int argc, char **argv);
    void setDeviceSoftening(float softening);
    void allocateNBodyArrays(float* vel[2], int numBytes);
    void deleteNBodyArrays(float* vel[2]);
    void integrateNbodySystem(float* newPos, float* newVel, 
                              float* oldPos, float* oldVel,
                              unsigned int pboOldPos, unsigned int pboNewPos,
                              float deltaTime, float damping, 
							  unsigned int numBodies, int offset, int length, int p, int q,
							  int bUsePBO);
    void copyArrayFromDevice(float* host, const float* device, unsigned int pbo, int numBytes);
    void copyArrayToDevice(float* device, const float* host, int numBytes);
    void registerGLBufferObject(unsigned int pbo);
    void unregisterGLBufferObject(unsigned int pbo);
	void threadSync();
}

namespace eqNbody
{
	
	void checkCUDAError(const char *msg)
	{
		cudaError_t err = cudaGetLastError();
		if( cudaSuccess != err) 
		{
			EQERROR << "CUDA error: " << msg << ": " << cudaGetErrorString( err) << std::endl;
			exit(EXIT_FAILURE);
		}                         
	}
	
	Controller::Controller() : _numBodies(0), _p(0), _q(0), _usePBO(true)
	{
		_dPos[0] = _dPos[1] = 0;
		_dVel[0] = _dVel[1] = 0;	
		
		_currentRead = 0;
		_currentWrite = 1;
	}
		
	bool Controller::init( int devID, const InitData& initData, float* hPos, bool usePBO, bool useGL )
	{		
		_numBodies	= initData.getNumBodies();
		_p			= initData.getP();
		_q			= initData.getQ();
		_damping	= initData.getDamping();
		
		_usePBO		= usePBO;
		_pointSize	= 1.0f;

		// Setup p and q properly
		if(_q * _p > 256)
		{
			_p = 256 / _q;
		}
		
		if (_q == 1 && _numBodies < _p)
		{
			_p = _numBodies;
		}
				
		// Setup the CUDA device
		if( devID == -1 ) {
			_devID = _getMaxGflopsDeviceId();
			EQWARN << "No CUDA device, using the fastest device: " << _devID << std::endl;
		}
		else {
			_devID = devID;
		}
		
		if(useGL) {
			cudaGLSetGLDevice( _devID );
		}
		else {
			cudaSetDevice( _devID );
		}
		
		cudaGetDevice(&_devID);
		cudaGetDeviceProperties(&_props, _devID);
		
		EQINFO << "Using CUDA device: " << _devID << std::endl;

		if (usePBO)
		{
			// create the position pixel buffer objects for rendering
			// we will actually compute directly from this memory in CUDA too
			glGenBuffers(2, (GLuint*)_pbo);   
			for (int i = 0; i < 2; ++i)
			{
				glBindBuffer(GL_ARRAY_BUFFER, _pbo[i]);
				glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * _numBodies, hPos, GL_DYNAMIC_DRAW);
				
				int size = 0;
				glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (GLint*)&size); 
				
				if ((unsigned)size != 4 * (sizeof(float) * _numBodies)) {
					EQERROR << "WARNING: Pixel Buffer Object allocation failed" << endl;
				}
			
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				registerGLBufferObject(_pbo[i]);
			}
		}
		else
		{
			allocateNBodyArrays(_dPos, _numBodies * sizeof( float) * 4);
		}
		
		allocateNBodyArrays(_dVel, _numBodies * sizeof( float) * 4);

		checkCUDAError("Controller::init");
		
		setSoftening(0.00125f);
		_renderer.init();
				
		return true;
	}
	
	bool Controller::exit()
	{
		deleteNBodyArrays(_dVel);
		
		if (_usePBO)
		{
			unregisterGLBufferObject(_pbo[0]);
			unregisterGLBufferObject(_pbo[1]);
			glDeleteBuffers(2, (const GLuint*)_pbo);
		}
		else
		{
			deleteNBodyArrays(_dPos);
		}
		
		return true;
	}
						
	void Controller::compute(const unsigned int frameID, const FrameData& fd, const eq::Range& range)
	{
		int offset	= range.start * _numBodies;
		int length	= ((range.end - range.start) * _numBodies) / _p;
			
		integrateNbodySystem(_dPos[_currentWrite], _dVel[_currentWrite], 
							 _dPos[_currentRead], _dVel[_currentRead],
							 _pbo[_currentWrite], _pbo[_currentRead],
							 fd.getTimeStep(), _damping, _numBodies, offset, length, _p, _q, 
							 (_usePBO ? 1 : 0));		
		
		checkCUDAError("Controller::run");
	}
	
	void Controller::draw(const FrameData& fd)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

		if(_usePBO) {
			_renderer.setPBO(_pbo[_currentRead], _numBodies);
		}
		else {
			_renderer.setPositions(fd.getPos(), _numBodies);
			_renderer.setPBO(0, _numBodies);
		}
		
		_renderer.setColors(fd.getCol(), _numBodies); // do this only on init
        _renderer.setSpriteSize(_pointSize);
		_renderer.draw(PARTICLE_SPRITES_COLOR);

		std::swap(_currentRead, _currentWrite);
	}
	
	void Controller::setSoftening(float softening)
	{
		setDeviceSoftening(softening);
	}
		
	void Controller::getArray(BodyArray array, DataProxy& proxy)
	{
		float* ddata = 0;		
		float* hdata = 0;
		unsigned int pbo = 0;
		
		unsigned int offset = proxy.getOffset();
						
		switch (array)
		{
			default:
			case BODYSYSTEM_POSITION:
				hdata = proxy.getPosition();
				ddata = _dPos[_currentRead];
				if (_usePBO) {
					pbo = _pbo[_currentRead];
				}
				break;
				
			case BODYSYSTEM_VELOCITY:
				hdata = proxy.getVelocity();
				ddata = _dVel[_currentRead];
				break;
		}

		copyArrayFromDevice(hdata+offset, ddata+offset, pbo, proxy.getNumBytes());
		proxy.markDirty();
	}
	
	void Controller::setArray(BodyArray array, const FrameData& fd)
	{
		unsigned int numBytes = fd.getNumBytes();
		
		switch (array)
		{
			default:
			case BODYSYSTEM_POSITION:
			{
				if (_usePBO)
				{
					unregisterGLBufferObject(_pbo[_currentRead]);
					glBindBuffer(GL_ARRAY_BUFFER, _pbo[_currentRead]);
					glBufferData(GL_ARRAY_BUFFER, numBytes, fd.getPos(), GL_DYNAMIC_DRAW);
					
					int size = 0;
					glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (GLint*)&size); 
					if ((unsigned)size != numBytes) {
						EQERROR << "WARNING: Pixel Buffer Object download failed. Size " << size << " does not match numBytes " << numBytes << std::endl;
					}
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					registerGLBufferObject(_pbo[_currentRead]);
				}
				else
				{
					copyArrayToDevice(_dPos[_currentRead], fd.getPos(), numBytes);
				}
			}
				break;

			case BODYSYSTEM_VELOCITY:
				copyArrayToDevice(_dVel[_currentRead], fd.getVel(), numBytes);
				break;
		}       
	}
		
	void Controller::synchronizeGPUThreads() const
	{
		threadSync();
	}
	
	int Controller::_getMaxGflopsDeviceId()
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
