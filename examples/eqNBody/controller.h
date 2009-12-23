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

#ifndef EQNBODY_NBODYSYSTEM_H
#define EQNBODY_NBODYSYSTEM_H

#include <driver_types.h>

#include "render_particles.h"
#include "sharedDataProxy.h"

namespace eqNbody
{
	class InitData;
	
	enum BodyArray 
    {
        BODYSYSTEM_POSITION,
        BODYSYSTEM_VELOCITY,
    };
		
    class Controller
    {
    public:
        Controller();

		bool init(const InitData& initData, float* hPos = NULL, bool usePBO=true);
		bool exit();
		
		void compute(const unsigned int frameID, const float timeStep, const eq::Range& range);
		void draw(float* pos, float* col);

		void setSoftening(float softening);
		
		void getArray(BodyArray array, SharedDataProxy& proxy);
		void setArray(BodyArray array, const float* pos, unsigned int numBytes);
		
    private:
		ParticleRenderer _renderer;
		
		unsigned int	_numBodies;
		unsigned int	_p;
		unsigned int	_q;
		float			_damping;		
		bool			_usePBO;

		float*			_dPos[2];		// position data on the GPU
		float*			_dVel[2];		// velocity data on the GPU
										
		unsigned int	_pbo[2];		// buffers
		unsigned int	_currentRead;	// current read buffer
		unsigned int	_currentWrite;	// current write buffer		
		float			_pointSize;
    };
}

#endif // EQNBODY_NBODYSYSTEM_H
