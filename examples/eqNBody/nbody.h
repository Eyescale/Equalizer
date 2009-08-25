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

#ifndef EQNBODY_NBODY_H
#define EQNBODY_NBODY_H

#include <cuda.h>

extern "C"
{
    void cudaInit(int argc, char **argv);
    void setDeviceSoftening(float softening);
    void allocateHostArrays(float** pos, float** vel, float** col, int nBytes);
    void deleteHostArrays(float* pos, float *vel, float *col);
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

#endif// EQNBODY_NBODY_H
