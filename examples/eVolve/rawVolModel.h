/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#ifndef EVOLVE_RAW_VOL_MODEL_H
#define EVOLVE_RAW_VOL_MODEL_H

#include <eq/eq.h>

namespace eVolve
{
#define COMPOSE_MODE_NEW

    /** Structure that contain actual dimensions of data that is 
        stored in volume texture.  

        It assumes that volume fills the cube [-1,-1,-1]..[1,1,1] and 
        the texture coordinates scaled to [0,0,0]..[1,1,1] but volume
        itself can be less then whole texture. Correct coordinates of
        volume stored in texture could be computed as:

        Xn = X * W
        Yn = Y * H
        Zn = Db + (Z - Do) * D

        so X and Y just scaled, but Z should be modifyed according to
        proper range.
    */
    struct DataInTextureDimensions
    {
        float W;    //!< Width  of data in texture (0..1]
        float H;    //!< Height of data in texture (0..1]
        float D;    //!< Depth  of data in texture (0..1]
        float Do;   //!< Depth offset (start of range)
        float Db;   //!< Depth border (necessary for preintegration)
    };

    /** Contain overal volume proportions relatively [-1,-1,-1]..[1,1,1] cube
    */
    struct VolumeScaling
    {
        float W;    //!< width  scale
        float H;    //!< height scale
        float D;    //!< depth  scale
    };

    struct VolumeInfo
    {
        GLuint                  volume;     //!< 3D texture ID
        GLuint                  preint;     //!< preintegration table texture
        VolumeScaling           volScaling; //!< Proportions of volume
        DataInTextureDimensions TD; //!< Data dimensions within volume texture
    };

    /** Load model to texture */
    class RawVolumeModel
    {
    public:
        RawVolumeModel( const std::string& filename );

        bool loadHeader( const float brightness, const float alpha );

        bool getVolumeInfo( VolumeInfo& info, const eq::Range& range );

        void releaseVolumeInfo( const eq::Range& range );

        const std::string&   getFileName()      const { return _filename;    };
              uint32_t       getResolution()    const { return _resolution;  };
        const VolumeScaling& getVolumeScaling() const { return _volScaling;  };

        void glewSetContext( GLEWContext* context ) { _glewContext = context; }

        GLEWContext* glewGetContext() { return _glewContext; }

    protected:

        bool _createVolumeTexture(
                                      GLuint&                  volume,
                                      DataInTextureDimensions& TD,
                                const eq::Range&               range );

    private:
        bool _lFailed( char* msg )
            { EQERROR << msg << std::endl; return false; }

        struct VolumePart
        {
            GLuint                  volume; //!< 3D texture ID
            DataInTextureDimensions TD;     //!< Data dimensions within volume
        };

        stde::hash_map< int32_t, VolumePart > _volumeHash; //!< 3D textures info

        bool         _headerLoaded;     //!< header is loaded successfully
        std::string  _filename;         //!< name of volume data file

        GLuint       _preintName;       //!< preintegration table texture

        uint32_t     _w;                //!< volume width
        uint32_t     _h;                //!< volume height
        uint32_t     _d;                //!< volume depth
        uint32_t     _resolution;       //!< max( _w, _h, _d ) of a model

        VolumeScaling _volScaling;      //!< Proportions of volume

        std::vector< uint8_t >  _TF;    //!< Transfer function

        GLEWContext*   _glewContext;    //!< OpenGL rendering context
    };

}

#endif // EVOLVE_RAW_VOL_MODEL_H
