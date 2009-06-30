
/* Copyright (c) 2007       Maxim Makhinya
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

#ifndef EVOLVE_RAW_VOL_MODEL_RENDERER_H
#define EVOLVE_RAW_VOL_MODEL_RENDERER_H

#include "rawVolModel.h"
#include "sliceClipping.h"
#include "glslShaders.h"


namespace eVolve
{

    class RawVolumeModelRenderer
    {
    public:
        RawVolumeModelRenderer( const std::string& filename,
                                const uint32_t     precision   = 1 );

        bool loadHeader( const float brightness, const float alpha )
        {
            return _rawModel.loadHeader( brightness, alpha );
        }

        const VolumeScaling& getVolumeScaling() const
        {
            return _rawModel.getVolumeScaling();
        }

        void glewSetContext( GLEWContext* context )
        {
            _glewContext = context;
            _rawModel.glewSetContext( context );
        }


        bool render( const eq::Range&       range,
                     const eq::Matrix4d&  modelviewM,
                     const eq::Matrix3d&  modelviewITM,
                     const eq::Matrix4f&  invRotationM  );

        void setPrecision( const uint32_t precision ){ _precision = precision; }
        void setOrtho( const uint32_t ortho )        { _ortho = ortho; }

        GLEWContext* glewGetContext() { return _glewContext; }
        bool loadShaders();

    private:
        void _putVolumeDataToShader( const VolumeInfo&     volumeInfo,
                                     const double          sliceDistance,
                                     const eq::Matrix4f& invRotationM );

        RawVolumeModel  _rawModel;      //!< volume data
        SliceClipper    _sliceClipper;  //!< frame clipping algorithm
        uint32_t        _precision;     //!< multiplyer for number of slices
        GLSLShaders     _shaders;       //!< GLSL shaders

        GLEWContext*    _glewContext;   //!< OpenGL rendering context

        bool            _ortho;         //!< ortogonal/perspective projection

    };

}
#endif // EVOLVE_RAW_VOL_MODEL_RENDERER_H

