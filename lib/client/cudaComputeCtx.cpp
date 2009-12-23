
/* Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
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

#include "cudaComputeCtx.h"
#include "pipe.h"

#ifdef EQ_USE_CUDA
# if defined __GNUC__
#  pragma GCC diagnostic ignored "-Wshadow"
# endif
# include <cuda_runtime_api.h>
# include <cuda_gl_interop.h>
# if defined __GNUC__
#  pragma GCC diagnostic warning "-Wshadow"
# endif
#endif

using namespace eq::base;
using namespace std;

namespace eq
{

    CUDAComputeCtx::CUDAComputeCtx( Pipe* parent ): ComputeCtx( parent )
    {
    }

    CUDAComputeCtx::~CUDAComputeCtx()
    {
    }

    //--------------------------------------------------------------------------
    // CUDA init
    //--------------------------------------------------------------------------
    bool CUDAComputeCtx::configInit( )
    {
#ifdef EQ_USE_CUDA
        cudaDeviceProp props;
        int device = (int)_pipe->getDevice();		
        int device_count = 0;
				
        // Setup the CUDA device
        if( (uint32_t)device == EQ_UNDEFINED_UINT32 ) {
            device = _getMaxGflopsDeviceId();
            EQWARN << "No CUDA device, using the fastest device: " << device << std::endl;
        }
		
        cudaGetDeviceCount( &device_count );
        EQINFO << "CUDA devices found: " << device_count << std::endl;
        EQASSERT( device_count > device );

        // We assume GL interop here, otherwise use cudaSetDevice( device );
        // Attention: this call requires a valid GL context!
        cudaGLSetGLDevice(device);
		
#ifdef WIN32
        // retrieve the CUDA device associated to the handle returned by 
        // WGL_NV_gpu_affinity().
        cudaWGLGetDevice(&device);
#else
        cudaGetDevice(&device);
#endif
        cudaGetDeviceProperties(&props, device);
		
        cudaError_t err = cudaGetLastError();
        if( cudaSuccess != err) 
        {
            ostringstream msg;
            msg << "CUDA initialization error: " << cudaGetErrorString( err) << std::endl;
            setErrorMessage( msg.str( ));				
            return false;
        }                         

        EQINFO << "Using CUDA device: " << device << std::endl;
        return true;
#else
        setErrorMessage( "Client library compiled without CUDA support" );
        return false;
#endif
    }

    //--------------------------------------------------------------------------
    // CUDA exit
    //--------------------------------------------------------------------------
    void CUDAComputeCtx::configExit()
    {
#ifdef EQ_USE_CUDA
        // Clean up all runtime-related resources associated with this thread.
        cudaThreadExit();
#else
        setErrorMessage( "Client library compiled without CUDA support" );
#endif
    }
    
    //--------------------------------------------------------------------------
    // CUDA exit
    //--------------------------------------------------------------------------
    int CUDAComputeCtx::_getMaxGflopsDeviceId()
    {		
#ifdef EQ_USE_CUDA
# if __DEVICE_EMULATION__
    return 0;
# else		
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
# endif
#else
        return 0;
#endif
    }
}
