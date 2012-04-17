
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
#include "ibMemBlock.h"
#ifdef EQ_INFINIBAND

#include <iba/ib_al.h>

namespace co

IBMemBlock::IBMemBlock()
        : _maxBufferSize( 0 )
        , _memoryRegion( 0 )
        , _localKey( 0 )
        , _remoteKey( 0 ){}

void IBMemBlock::close()
{
    if ( _memoryRegion )
    {
       ib_api_status_t status = ib_dereg_mr( _memoryRegion );
       _memoryRegion = 0;
    }
    _localKey = 0;
    _remoteKey = 0;
}

bool IBMemBlock::create( ib_pd_handle_t  protectionDomain, 
                         const uint32_t  bufferBlockSize )
{
    _memoryRegion     = 0 ;
    _localKey         = 0 ;
    _remoteKey        = 0 ;
    _protectionDomain = protectionDomain;
    _maxBufferSize    = bufferBlockSize;

    // init buffer base
    buf.resize( _maxBufferSize );

    /* Create and register a memory region */
    // Information describing the memory region to register
    ib_mr_create_t  memoryRegionCreate;
    memoryRegionCreate.length      = _maxBufferSize;
    memoryRegionCreate.vaddr       = buf.getData();
    memoryRegionCreate.access_ctrl = IB_AC_RDMA_WRITE | 
                                     IB_AC_LOCAL_WRITE;
        
    ib_api_status_t     ibStatus ;   

    // Registers a virtual memory region with a channel adapter.
    ibStatus = ib_reg_mem( _protectionDomain, &memoryRegionCreate, 
                           &_localKey, &_remoteKey, &_memoryRegion );

    if ( ibStatus != IB_SUCCESS )
    {
        LBERROR << "Can't create memory region !!!" << std::endl;
        return false;
    }
    return true;
}

}
#endif //EQ_INFINIBAND
