
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

#ifndef MASS_VOL__CAMERAANIMATION_H
#define MASS_VOL__CAMERAANIMATION_H

#include <msv/types/vmmlTypes.h>

namespace massVolVis
{

    /**
     * Loads sequence of camera positions and interpolates them on a per-frame
     * basis.
     */
    class CameraAnimation
    {
    public:
        struct Step;

        CameraAnimation() : _curStep( 0 ), _curFrame( 0 ) {}

        bool loadAnimation( const std::string& fileName );

        bool isValid() const { return !_steps.empty(); }

        Step getNextStep();

        uint32_t getCurrentFrame() { return _curFrame; }

        const Vector3f& getModelRotation() const { return _modelRotation;}

        struct Step
        {
            Step()
                : frame( 0 )
                , position( Vector3f( .0f, .0f, -1.0f ))
                , rotation( Vector3f( .0f, .0f,   .0f )){}

            Step( int frame_, const Vector3f& position_,
                              const Vector3f& rotation_  )
                : frame( frame_ )
                , position( position_ ),
                  rotation( rotation_ ){}

            int frame;
            Vector3f position;
            Vector3f rotation;
        };

    private:
        Vector3f            _modelRotation;
        std::vector< Step > _steps;
        uint32_t            _curStep;
        int32_t             _curFrame;
    };

}

#endif // MASS_VOL__CAMERAANIMATION_H

