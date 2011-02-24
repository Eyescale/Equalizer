/*
 * Copyright 1993-2006 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:   
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and 
 * international Copyright laws.  
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE 
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR 
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.   
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOURCE CODE.  
 *
 * U.S. Government End Users.  This source code is a "commercial item" as 
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of 
 * "commercial computer software" and "commercial computer software 
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995) 
 * and is provided to the U.S. Government only as a commercial end item.  
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through 
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the 
 * source code with only those rights set forth herein.
 */

#include "nbody_kernel.cu"

#include <cstdlib>
#include <cstdio>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#endif

#include <cuda_gl_interop.h>

extern "C"
{

void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) 
    {
        fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }                         
}

void setDeviceSoftening(float softening)
{
    float softeningSq = softening*softening;
    cudaMemcpyToSymbol("softeningSquared", &softeningSq, sizeof(float), 0, cudaMemcpyHostToDevice);
}

void allocateHostArrays(float** pos, float** vel, float** col, int numBytes)
{
	unsigned int flags = cudaHostAllocDefault; // cudaHostAllocWriteCombined
								
	cudaHostAlloc( (void**)pos, numBytes, flags);
	cudaHostAlloc( (void**)vel, numBytes, flags);
	cudaHostAlloc( (void**)col, numBytes, flags);
}

void deleteHostArrays(float* pos, float *vel, float *col)
{
	cudaFreeHost(pos);
	cudaFreeHost(vel);
	cudaFreeHost(col);
}

void allocateNBodyArrays(float* vel[2], int numBytes)
{    
    cudaMalloc((void**)&vel[0], numBytes);
    cudaMalloc((void**)&vel[1], numBytes);
}

void deleteNBodyArrays(float* vel[2])
{
    cudaFree((void**)vel[0]);
    cudaFree((void**)vel[1]);
}

void copyArrayFromDevice(float* host, const float* device, unsigned int pbo, int numBytes)
{   
    if (pbo) {
        cudaGLMapBufferObject((void**)&device, pbo);
	}
   
    cudaMemcpy(host, device, numBytes, cudaMemcpyDeviceToHost);

    if (pbo) {
        cudaGLUnmapBufferObject(pbo);
	}
}

void copyArrayToDevice(float* device, const float* host, int numBytes)
{
    cudaMemcpy(device, host, numBytes, cudaMemcpyHostToDevice);
}

void registerGLBufferObject(unsigned int pbo)
{
    cudaGLRegisterBufferObject(pbo);
}

void unregisterGLBufferObject(unsigned int pbo)
{
    cudaGLUnregisterBufferObject(pbo);
}

void threadSync() { cudaThreadSynchronize(); }

void integrateNbodySystem(float* newPos, float* newVel, 
                     float* oldPos, float* oldVel, 
                     unsigned int pboNewPos, unsigned int pboOldPos, 
                     float deltaTime, float damping, 
                     unsigned int numBodies, int offset, int length, int p, int q,
                     int bUsePBO)
{
    int sharedMemSize = p * q * sizeof(float4); // 4 floats for pos
    
    dim3 threads(p,q,1);
    dim3 grid(length, 1, 1);

    if (bUsePBO)
    {
        cudaGLMapBufferObject((void**)&oldPos, pboOldPos);
        cudaGLMapBufferObject((void**)&newPos, pboNewPos);
    }

    // execute the kernel:

    // When the numBodies / thread block size is < # multiprocessors 
    // (16 on G80), the GPU is underutilized. For example, with 256 threads per
    // block and 1024 bodies, there will only be 4 thread blocks, so the 
    // GPU will only be 25% utilized.  To improve this, we use multiple threads
    // per body.  We still can use blocks of 256 threads, but they are arranged
    // in q rows of p threads each.  Each thread processes 1/q of the forces 
    // that affect each body, and then 1/q of the threads (those with 
    // threadIdx.y==0) add up the partial sums from the other threads for that 
    // body.  To enable this, use the "--p=" and "--q=" command line options to
    // this example.  e.g.: "nbody.exe --n=1024 --p=64 --q=4" will use 4 
    // threads per body and 256 threads per block. There will be n/p = 16 
    // blocks, so a G80 GPU will be 100% utilized.

    // We use a bool template parameter to specify when the number of threads 
    // per body is greater than one, so that when it is not we don't have to 
    // execute the more complex code required!
    if (threads.y == 1)
    {
        integrateBodies<false><<< grid, threads, sharedMemSize >>>
            ((float4*)newPos, (float4*)newVel,
            (float4*)oldPos, (float4*)oldVel,
            deltaTime, damping, numBodies, offset);
    }
    else
    {
        integrateBodies<true><<< grid, threads, sharedMemSize >>>
            ((float4*)newPos, (float4*)newVel,
            (float4*)oldPos, (float4*)oldVel,
            deltaTime, damping, numBodies, offset);
    }
    
    if (bUsePBO)
    {
        cudaGLUnmapBufferObject(pboNewPos);
        cudaGLUnmapBufferObject(pboOldPos);
    }
}


}
