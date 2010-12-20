
/*
 * Copyright (c) 2009, Philippe Robert <philippe.robert@gmail.com> 
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQNBODY_INITDATA_H
#define EQNBODY_INITDATA_H

#include <eq/eq.h>

namespace eqNbody
{
    class InitData : public co::Object
    {
    public:
        InitData();
        virtual ~InitData();

        void setFrameDataID( const eq::uint128_t id )   { _frameDataID = id; }
        const eq::uint128_t& getFrameDataID() const { return _frameDataID; }

        float getDamping() const { return _damping; }
        uint32_t getNumBodies() const { return _numBodies; }
        uint32_t getP() const { return _p; }
        uint32_t getQ() const { return _q; }
        
    protected:
        virtual void getInstanceData( co::DataOStream& os );
        virtual void applyInstanceData( co::DataIStream& is );

    private:
        eq::uint128_t   _frameDataID;
        uint32_t    _numBodies;     // number of bodies in simulation
        uint32_t    _p;             // CUDA thread parameter p
        uint32_t    _q;             // CUDA thread parameter q
        float       _damping;       // damping factor
    };
}

#endif // EQNBODY_INITDATA_H

