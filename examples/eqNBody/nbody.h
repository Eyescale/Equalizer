
/*
 * Copyright (c) 2009, Philippe Robert <probert@eyescale.ch> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EQNBODY_NBODY_H
#define EQNBODY_NBODY_H

#include <cuda.h>

extern "C"
{
    void cudaInit( int argc, char **argv );
    void setDeviceSoftening( float softening );
    void allocateHostArrays( float** pos, float** vel, float** col, int nBytes );
    void deleteHostArrays( float* pos, float *vel, float *col );
    void allocateNBodyArrays( float* vel[2], int numBytes );
    void deleteNBodyArrays( float* vel[2] );
    void integrateNbodySystem( float* newPos, float* newVel, 
                               float* oldPos, float* oldVel,
                               unsigned int pboOldPos, unsigned int pboNewPos,
                               float deltaTime, float damping, 
                               unsigned int numBodies, int offset, int length,
                              int p, int q, int bUsePBO );
    void copyArrayFromDevice( float* host, const float* device, unsigned int pbo, 
                              int numBytes );
    void copyArrayToDevice( float* device, const float* host, int numBytes );
    void registerGLBufferObject( unsigned int pbo );
    void unregisterGLBufferObject( unsigned int pbo );
    void threadSync();
}

#endif// EQNBODY_NBODY_H
