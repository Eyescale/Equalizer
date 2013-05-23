
/* Copyright (c) 2009, Maxim Makhinya
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

#include "cameraAnimation.h"

#include <iostream>
#include <fstream>

namespace massVolVis
{

CameraAnimation::Step CameraAnimation::getNextStep()
{
    LBASSERT( _steps.size() > 0 );
    LBASSERT( _curStep < _steps.size() );

    if( _steps.size() == 0 )
        return Step();

    if( _steps.size() == 1 )
        return _steps[ _curStep ];

    LBASSERT( _curStep < _steps.size()-1 );

    ++_curFrame;
    if( _curFrame > _steps[_curStep+1].frame )
    {
        if( _curStep == _steps.size()-2 )
        {
            _curFrame = 1;
            _curStep  = 0;
        }
        else
            ++_curStep;
    }
    //else
    const Step& curStep  = _steps[ _curStep   ];
    const Step& nextStep = _steps[ _curStep+1 ];

    if( _curFrame < curStep.frame )
        _curFrame = curStep.frame+1;

    const float interval  = float( nextStep.frame - curStep.frame );
    const float curCoeff  = ( nextStep.frame - _curFrame ) / interval;
    const float nextCoeff = ( _curFrame - curStep.frame ) / interval;

    Step result( _curFrame,
                 curStep.position * curCoeff + nextStep.position * nextCoeff,
                 curStep.rotation * curCoeff + nextStep.rotation * nextCoeff );

    return result;
}


bool CameraAnimation::loadAnimation( const std::string& fileName )
{
    _steps.clear();

    if( fileName.empty( ))
        return false;

    std::ifstream file;
    file.open( fileName.c_str( ));
    if( !file )
    {
        LBERROR << "Path file could not be opened" << std::endl;
        return false;
    }

    // read model pre-rotation
    file >> _modelRotation.x();
    file >> _modelRotation.y();
    file >> _modelRotation.z();

    const float m = static_cast<float>(M_PI_2) / 90.f;
    _modelRotation *= m;

    uint32_t count = 0;
    float v[7];
    int frameNum = 0;
    while ( !file.eof( ))
    {
        file >> v[count++];
        if( count == 7 )
        {
            count = 0;
            frameNum += LB_MAX( static_cast<int>( v[0] ), 1 );

            _steps.push_back( Step( frameNum,
                             Vector3f(  v[1]  , v[2]  , v[3]   ),
                             Vector3f( -v[5]*m, v[4]*m, v[6]*m )));
        }
    }
    file.close();

    return true;
}

}
