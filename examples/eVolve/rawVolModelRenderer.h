/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_RAW_VOL_MODEL_RENDERER_H
#define EVOLVE_RAW_VOL_MODEL_RENDERER_H

#include "rawVolModel.h"
#include "sliceClipping.h"


namespace eVolve
{

    class RawVolumeModelRenderer
    {
    public:
        RawVolumeModelRenderer( const std::string& filename, 
                                const uint32_t     precision = 1 );

        bool loadHeader( const float brightness, const float alpha )
        {
            return _rawModel.loadHeader( brightness, alpha );
        }

        const VolumeScaling& getVolumeScaling() const
        {
            return _rawModel.getVolumeScaling();
        };

        void glewSetContext( GLEWContext* context )
        {
            _glewContext = context;
            _rawModel.glewSetContext( context );
        }


        bool Render(    const eq::Range&        range,
                        const vmml::Matrix4d&   modelviewM,
                        const vmml::Matrix3d&   modelviewITM );

        void SetPrecision( const uint32_t precision ){ _precision = precision; }

        GLEWContext* glewGetContext() { return _glewContext; }
        bool LoadShaders();

    private:
        void _putVolumeDataToShader( const VolumeInfo& volumeInfo,
                                     const double      sliceDistance );

        RawVolumeModel  _rawModel;      //!< volume data
        SliceClipper    _sliceClipper;  //!< frame clipping algorithm
        uint32_t        _precision;     //!< multiplyer for number of slices
        GLhandleARB     _shader;        //!< GLSL shaders
        bool            _shadersLoaded;

        GLEWContext*    _glewContext;   //!< OpenGL rendering context
    };

}
#endif // EVOLVE_RAW_VOL_MODEL_RENDERER_H