/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

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
                                const uint32_t     precision   = 1,
                                const bool         perspective = true );

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
                     const vmml::Matrix4d&  modelviewM,
                     const vmml::Matrix3d&  modelviewITM,
                     const vmml::Matrix4f&  invRotationM  );

        void setPrecision( const uint32_t precision ){ _precision = precision; }

        GLEWContext* glewGetContext() { return _glewContext; }
        bool loadShaders();

    private:
        void _putVolumeDataToShader( const VolumeInfo&     volumeInfo,
                                     const double          sliceDistance,
                                     const vmml::Matrix4f& invRotationM );

        RawVolumeModel  _rawModel;      //!< volume data
        SliceClipper    _sliceClipper;  //!< frame clipping algorithm
        uint32_t        _precision;     //!< multiplyer for number of slices
        GLSLShaders     _shaders;       //!< GLSL shaders

        GLEWContext*    _glewContext;   //!< OpenGL rendering context

        bool            _perspective;   //!< perspective/ortogonal projection

    };

}
#endif // EVOLVE_RAW_VOL_MODEL_RENDERER_H

