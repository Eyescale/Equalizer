
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
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

#include "sharedDataProxy.h"
#include "client.h"

namespace eqNbody
{
    
    SharedDataProxy::SharedDataProxy() : _offset(0), _numBytes(0)
    {            
        _hPos = NULL;
        _hVel = NULL;
        _hCol = NULL;
    }
        
    void SharedDataProxy::serialize( co::DataOStream& os,
                                     const uint64_t dirtyBits )
    {
        co::Serializable::serialize( os, dirtyBits );
        
        if( dirtyBits & DIRTY_DATA )
        {
            LBASSERT(_hPos != NULL);
            LBASSERT(_hVel != NULL);

            os << _offset << _numBytes
               << co::Array< void >( _hPos+_offset, _numBytes )
               << co::Array< void >( _hVel+_offset, _numBytes );
            //(_hCol+_offset, _numBytes);
        }        
    }
    
    void SharedDataProxy::deserialize( co::DataIStream& is,
                                       const uint64_t dirtyBits )
    {
        co::Serializable::deserialize( is, dirtyBits );

        if( dirtyBits & DIRTY_DATA )
        {
            LBASSERT(_hPos != NULL);
            LBASSERT(_hVel != NULL);

            is >> _offset >> _numBytes
               >> co::Array< void >( _hPos+_offset, _numBytes )
               >> co::Array< void >( _hVel+_offset, _numBytes );
            //(_hCol+_offset, _numBytes);
        }        
    }
        
    void SharedDataProxy::init(const unsigned int offset, const unsigned int numBytes, float *pos, float *vel, float *col)
    {
        _offset        = offset;
        _numBytes    = numBytes;

        _hPos        = pos;
        _hVel        = vel;
        _hCol        = col;
        
        setDirty( DIRTY_DATA );
    }

    void SharedDataProxy::init(float *pos, float *vel, float *col)
    {
        _hPos        = pos;
        _hVel        = vel;
        _hCol        = col;
        
        setDirty( DIRTY_DATA );
    }
    
    void SharedDataProxy::exit()
    {
        _offset      = 0;
        _numBytes    = 0;
        
        _hPos        = NULL;
        _hVel        = NULL;
        _hCol        = NULL;
    }
    
    void SharedDataProxy::markDirty()
    {
        setDirty( DIRTY_DATA );
    }
    
}

