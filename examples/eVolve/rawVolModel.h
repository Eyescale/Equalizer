
#ifndef RAW_VOL_MODEL_H
#define RAW_VOL_MODEL_H

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
    struct VolumeScales
    {
        float W;
        float H;
        float D;
    };

    struct Volume
    {
        GLuint                  volume; //!< 3D texture ID
        DataInTextureDimensions TD; //!< Data dimensions within volume texture 
        VolumeScales            volScales; //!< Proportions of volume
    };

    /** Load model to texture */
    class RawVolumeModel
    {
    public:
    
        RawVolumeModel( const std::string& data );
    
        bool createTextures( GLuint &volume, GLuint &preint, 
                             const eq::Range& range );

        const eq::Range&    getCurRange()      const { return _cRange;      };
        const std::string&  getFileName()      const { return _fileName;    };
              uint32_t      getResolution()    const { return _resolution;  };
              bool          getLoadingResult() const { return _lastSuccess; };

    //Data
        DataInTextureDimensions TD; //!< Data dimensions within volume texture 
        VolumeScales            volScales; //!< Proportions of volume
    
    private:
        
        bool lSuccess() { return _lastSuccess=true;  }
        bool lFailed( char* msg )  
                { EQERROR << msg << std::endl; return _lastSuccess=false; }

        eq::Range  _cRange;        //!< Current Range
        bool       _lastSuccess;   //!< Result of last loading

        uint32_t     _resolution;   //!< max(w,h,d) of a model
        std::string  _fileName;
    };

}

#endif // RAW_VOL_MODEL_H
