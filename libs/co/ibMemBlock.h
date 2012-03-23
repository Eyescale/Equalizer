
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
#include <eq/base/base.h>
#ifdef EQ_INFINIBAND
#ifndef CO_IBMEMBLOCK_H
#define CO_IBMEMBLOCK_H

#include <eq/base/buffer.h>
#include <iba/ib_types.h>

namespace co
{
class IBMemBlock
{
public:
    IBMemBlock();
    virtual ~IBMemBlock(){ close(); }
    void close();

    /* create an memory region associate with the given protection domain */
    bool create( ib_pd_handle_t  protectionDomain, 
                 const uint32_t  bufferBlockSize   );
    
    // get the local access key associated with this registered 
    // memory region
    uint32_t getLocalKey( )  const { return _localKey; };

    // get key that may be used by a remote end-point when performing
    // RDMA or atomic operations to this registered memory region
    uint32_t getRemoteKey( ) const { return _remoteKey; }

    ib_mr_handle_t getMemoryRegion( )  const { return _memoryRegion; }
            
    uintptr_t getVaddr( ) const 
           { return ( uintptr_t ) buf.getData(); }

    eq::lunchbox::Buffer<void *> buf;

private:
    // Collection of memory pages within the local HCA's memory
    ib_mr_handle_t  _memoryRegion;
    uint32_t        _localKey;
    uint32_t        _remoteKey;
    uint32_t        _maxBufferSize;
    ib_pd_handle_t  _protectionDomain;

};
}
#endif //CO_IBCONNECTION_H 
#endif //EQ_INFINIBAND
