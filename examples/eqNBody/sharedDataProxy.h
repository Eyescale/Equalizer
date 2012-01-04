
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

#ifndef EQNBODY_DATAPROXY_H
#define EQNBODY_DATAPROXY_H

#include <eq/eq.h>

namespace eqNbody
{
    class SharedDataProxy : public co::Serializable
    {
    public:

        SharedDataProxy();

        void init( const unsigned int offset, const unsigned int numBytes, 
                   float *pos, float *vel, float *col );
        void init( float *pos, float *vel, float *col );
        void exit();    
        
        void markDirty();

        unsigned int getOffset() const {return _offset;}
        unsigned int getNumBytes() const {return _numBytes;}
        
        float* getPosition() const {return _hPos;}
        float* getVelocity() const {return _hVel;}
        
    protected:
        virtual void serialize( co::DataOStream& os, 
                                const uint64_t dirtyBits );
        virtual void deserialize( co::DataIStream& is, 
                                  const uint64_t dirtyBits );
        virtual ChangeType getChangeType() const { return UNBUFFERED; }
        enum DirtyBits
        {
            DIRTY_DATA   = co::Serializable::DIRTY_CUSTOM << 0
        };

    private:
        unsigned int _offset;    // offset into the frameData's memory chunk
        unsigned int _numBytes;  // number of bytes to be written
        
        float*    _hPos;         // frameData's position data on the host
        float*    _hVel;         // frameData's velocity data on the host
        float*    _hCol;         // frameData's color data on the host
    };
}

#endif // EQNBODY_DATAPROXY_H

