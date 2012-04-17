
/* Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com>
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#include "cudaContext.h"
#include "pipe.h"
#include "error.h"

#ifdef WIN32_API
# include "wgl/pipe.h"
#endif

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

namespace eq
{

    CUDAContext::CUDAContext( Pipe* parent ): ComputeContext( parent )
    {
    }

    CUDAContext::~CUDAContext()
    {
    }
#ifdef WIN32_API
    WGLEWContext* CUDAContext::wglewGetContext() 
    {
        return static_cast<wgl::Pipe*>(getPipe()->getSystemPipe())->wglewGetContext(); 
    }
#endif
    //--------------------------------------------------------------------------
    // CUDA init
    //--------------------------------------------------------------------------
    bool CUDAContext::configInit( )
    {
#ifdef EQ_USE_CUDA
        cudaDeviceProp props;
        uint32_t device = getPipe()->getDevice();

        // Setup the CUDA device
        if( device == LB_UNDEFINED_UINT32 )
        {
            device = _getFastestDeviceID();
            LBWARN << "No CUDA device, using the fastest device: " << device
                   << std::endl;
        }

        int device_count = 0;
        cudaGetDeviceCount( &device_count );
        LBINFO << "CUDA devices found: " << device_count << std::endl;
        LBASSERT( static_cast< uint32_t >( device_count ) > device );
        if( static_cast< uint32_t >( device_count ) <= device )
        {
            LBWARN << "Not enough cuda devices, requested device " << device
                   << " of " << device_count << std::endl;
            setError( ERROR_CUDACONTEXT_DEVICE_NOTFOUND );
            return false;
        }

        // We assume GL interop here, otherwise use cudaSetDevice( device );
        // Attention: this call requires a valid GL context!
        cudaGLSetGLDevice( device );

        int usedDevice = static_cast< int >( device );
#ifdef _WIN32

        HGPUNV handle = 0;

        if( !WGLEW_NV_gpu_affinity )
        {
            LBWARN <<"WGL_NV_gpu_affinity unsupported, ignoring device setting"
                   << std::endl;
            return true;
        }

        if( !wglEnumGpusNV( device, &handle ))
        {
           LBWARN << "wglEnumGpusNV failed : " << lunchbox::sysError << std::endl;
            return false;
        }

        cudaWGLGetDevice( &usedDevice, handle );
#else
        cudaGetDevice( &usedDevice );
#endif
        LBASSERT( device == static_cast< uint32_t >( device ));
        cudaGetDeviceProperties( &props, usedDevice );

        cudaError_t err = cudaGetLastError();
        if( cudaSuccess != err) 
        {
            LBWARN << "CUDA initialization error: "
                   << cudaGetErrorString( err ) << std::endl;
            setError( ERROR_CUDACONTEXT_INIT_FAILED );
            return false;
        }                         

        LBINFO << "Using CUDA device: " << device << std::endl;
        return true;
#else
        setError( ERROR_CUDACONTEXT_MISSING_SUPPORT );
        return false;
#endif
    }

    //--------------------------------------------------------------------------
    // CUDA exit
    //--------------------------------------------------------------------------
    void CUDAContext::configExit()
    {
#ifdef EQ_USE_CUDA
        // Clean up all runtime-related resources associated with this thread.
        cudaThreadExit();
#else
        setError( ERROR_CUDACONTEXT_MISSING_SUPPORT );
#endif
    }
    
    //--------------------------------------------------------------------------
    // CUDA exit
    //--------------------------------------------------------------------------
    int CUDAContext::_getFastestDeviceID()
    {
#ifdef EQ_USE_CUDA
# if __DEVICE_EMULATION__
        return 0;
# else
        int device_count = 0;
        cudaGetDeviceCount( &device_count );
        
        cudaDeviceProp device_properties;
        int max_gflops_device = 0;

        int current_device = 0;
        cudaGetDeviceProperties( &device_properties, current_device );
        int max_gflops = device_properties.multiProcessorCount * 
                         device_properties.clockRate;

        ++current_device;
        while( current_device < device_count )
        {
            cudaGetDeviceProperties( &device_properties, current_device );
            const int gflops = device_properties.multiProcessorCount * 
                               device_properties.clockRate;
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
