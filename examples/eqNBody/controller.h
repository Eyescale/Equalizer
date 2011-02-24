
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

#ifndef EQNBODY_NBODYSYSTEM_H
#define EQNBODY_NBODYSYSTEM_H

#include <driver_types.h>

#include "render_particles.h"
#include "sharedDataProxy.h"

namespace eqNbody
{
    class InitData;
    
    enum BodyArray 
    {
        BODYSYSTEM_POSITION,
        BODYSYSTEM_VELOCITY,
    };
        
    class Controller
    {
    public:
        Controller( const GLEWContext* const glewContext );
        bool init( const InitData& initData, float* hPos = NULL, 
                   bool usePBO=true );
        bool exit();

        void compute( const float timeStep, const eq::Range& range );
        void draw( float* pos, float* col );

        void setSoftening( float softening );
        
        void getArray( BodyArray array, SharedDataProxy& proxy );
        void setArray( BodyArray array, const float* pos, unsigned int numBytes );

        const GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        ParticleRenderer _renderer;
        const GLEWContext* _glewContext;

        unsigned int _numBodies;
        unsigned int _p;
        unsigned int _q;
        float        _damping;
        bool         _usePBO;

        float*       _dPos[2];      // position data on the GPU
        float*       _dVel[2];      // velocity data on the GPU
                                        
        unsigned int _pbo[2];       // buffers
        unsigned int _currentRead;  // current read buffer
        unsigned int _currentWrite; // current write buffer
        float        _pointSize;
    };
}

#endif // EQNBODY_NBODYSYSTEM_H
