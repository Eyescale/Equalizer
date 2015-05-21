
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

#include "controller.h"
#include "initData.h"
#include "nbody.h"

#include <eq/gl.h>

#include <cuda.h>
#include <cuda_gl_interop.h>

namespace eqNbody
{
Controller::Controller( const GLEWContext* const glewContext ) 
    : _renderer( glewContext )
    , _glewContext( glewContext )
    , _numBodies( 0 )
    , _p( 0 )
    , _q( 0 )
    , _usePBO( true )
{
   _dPos[0] = _dPos[1] = 0;
   _dVel[0] = _dVel[1] = 0;
    _currentRead = 0;
    _currentWrite = 1;
}

bool Controller::init( const InitData& initData, float* hPos, bool usePBO )
{        
    _numBodies = initData.getNumBodies();
    _p         = initData.getP();
    _q         = initData.getQ();
    _damping   = initData.getDamping();
    _usePBO    = usePBO;
    _pointSize = 1.0f;

    // Setup p and q properly
    if( _q * _p > 256 )
        _p = 256 / _q;

    if ( _q == 1 && _numBodies < _p )
        _p = _numBodies;

    if( usePBO )
    {
        // create the position pixel buffer objects for rendering
        // we will actually compute directly from this memory in CUDA too
        glGenBuffers(2, (GLuint*)_pbo);   
        for (int i = 0; i < 2; ++i)
        {
            glBindBuffer( GL_ARRAY_BUFFER, _pbo[i] );
            glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(float) * _numBodies, 
                          hPos, GL_DYNAMIC_DRAW);
            
            int size = 0;
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (GLint*)&size); 
            if ((unsigned)size != 4 * (sizeof(float) * _numBodies)) {
                LBERROR << "WARNING: Pixel Buffer Object allocation failed" 
                        << std::endl;
            }

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            registerGLBufferObject(_pbo[i]);
        }
    }
    else
    {
        allocateNBodyArrays(_dPos, _numBodies * sizeof( float) * 4);
    }
    
    allocateNBodyArrays(_dVel, _numBodies * sizeof( float) * 4);
    setSoftening(0.00125f);
    _renderer.init();

    return true;
}
bool Controller::exit()
{
    deleteNBodyArrays(_dVel);
    
    if (_usePBO)
    {
        unregisterGLBufferObject(_pbo[0]);
        unregisterGLBufferObject(_pbo[1]);
        glDeleteBuffers(2, (const GLuint*)_pbo);
    }
    else
    {
        deleteNBodyArrays(_dPos);
    }
    
    return true;
}
                    
void Controller::compute(const float timeStep, const eq::Range& range)
{
    int offset    = range.start * _numBodies;
    int length    = ((range.end - range.start) * _numBodies) / _p;
        
    integrateNbodySystem(_dPos[_currentWrite], _dVel[_currentWrite], 
                         _dPos[_currentRead], _dVel[_currentRead],
                         _pbo[_currentWrite], _pbo[_currentRead],
                         timeStep, _damping, _numBodies, offset, length,
                         _p, _q, (_usePBO ? 1 : 0));
}

void Controller::draw(float* pos, float* col)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -50.0f );
    
    if(_usePBO) {
        _renderer.setPBO(_pbo[_currentRead], _numBodies);
    }
    else {
        _renderer.setPositions(pos, _numBodies);
        _renderer.setPBO(0, _numBodies);
    }
    
    _renderer.setColors(col, _numBodies); // do this only on init
    _renderer.setSpriteSize(_pointSize);
    _renderer.draw(PARTICLE_SPRITES_COLOR);

    std::swap(_currentRead, _currentWrite);
}

void Controller::setSoftening(float softening)
{
    setDeviceSoftening(softening);
}
    
void Controller::getArray(BodyArray array, SharedDataProxy& proxy)
{
    float* ddata = 0;
    float* hdata = 0;
    unsigned int pbo = 0;
    
    const unsigned int offset = proxy.getOffset();

    switch (array)
    {
        default:
        case BODYSYSTEM_POSITION:
            hdata = proxy.getPosition();
            ddata = _dPos[ _currentRead ];
            if (_usePBO) {
                pbo = _pbo[ _currentRead ];
            }
            break;

        case BODYSYSTEM_VELOCITY:
            hdata = proxy.getVelocity();
            ddata = _dVel[ _currentRead ];
            break;
    }

    copyArrayFromDevice( hdata + offset, ddata + offset, pbo, 
                         proxy.getNumBytes());
    proxy.markDirty();
}

void Controller::setArray( BodyArray array, const float* pos, 
                           unsigned int numBytes )
{        
    switch (array)
    {
        default:
        case BODYSYSTEM_POSITION:
        {
            if ( !_usePBO )
            {
                copyArrayToDevice( _dPos[ _currentRead ], pos, numBytes );
                return;
            }

            unregisterGLBufferObject( _pbo[ _currentRead ] );
            glBindBuffer(GL_ARRAY_BUFFER, _pbo[_currentRead]);
            glBufferData(GL_ARRAY_BUFFER, numBytes, pos, GL_DYNAMIC_DRAW);
            
            int size = 0;
            glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, 
                                    ( GLint* )&size ); 
            if ((unsigned)size != numBytes) 
            {
                LBERROR << " WARNING : Pixel Buffer Object download failed."  
                        <<  " Size " << size << " does not match numBytes " 
                        << numBytes << std::endl;
            }
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            registerGLBufferObject( _pbo[ _currentRead ] );
            break;
        }
        
        case BODYSYSTEM_VELOCITY:
            copyArrayToDevice( _dVel[ _currentRead ], pos, numBytes );
            break;
    }       
}

}
