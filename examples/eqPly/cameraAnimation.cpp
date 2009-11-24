
/* Copyright (c) 2009, Maxim Makhinya
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

#include "cameraAnimation.h"
#include <eq/base/debug.h>

#include <iostream>
#include <fstream>

namespace eqPly
{

CameraAnimation::Step CameraAnimation::getNextStep()
{
    EQASSERT( _steps.size() > 0 );
    EQASSERT( _curStep < _steps.size() );

    if( _steps.size() == 0 )
        return Step();

    if( _steps.size() == 1 )
        return _steps[ _curStep ];

    EQASSERT( _curStep < _steps.size()-1 );

    _curFrame++;
    if( _curFrame > _steps[_curStep+1].frame )
    {
        if( _curStep == _steps.size()-2 )
        {
            _curFrame = 1;
            _curStep  = 0;
        }else
            _curStep++;
    }
    //else
    const Step& curStep  = _steps[ _curStep   ];
    const Step& nextStep = _steps[ _curStep+1 ];

    if( _curFrame < curStep.frame )
        _curFrame = curStep.frame+1;

    const float interval  = nextStep.frame - curStep.frame;
    const float curCoeff  = ( nextStep.frame - _curFrame ) / interval;
    const float nextCoeff = ( _curFrame - curStep.frame ) / interval;

    Step result( _curFrame,
                 curStep.translation*curCoeff + nextStep.translation*nextCoeff,
                 curStep.rotation   *curCoeff + nextStep.rotation   *nextCoeff);

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
        EQERROR << "Path file could not be opened" << std::endl;
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
            frameNum += EQ_MAX( static_cast<int>( v[0] ), 1 );

            _steps.push_back( Step( frameNum,
                             eq::Vector3f(  v[1]  , v[2]  , v[3]   ),
                             eq::Vector3f( -v[5]*m, v[4]*m, v[6]*m )));
        }
    }
    file.close();

    return true;
}

}
