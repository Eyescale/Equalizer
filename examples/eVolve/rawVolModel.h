
/* Copyright (c) 2007       Maxim Makhinya
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

#ifndef EVOLVE_RAW_VOL_MODEL_H
#define EVOLVE_RAW_VOL_MODEL_H

#include <eq/eq.h>

namespace eVolve
{

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

    /** Contain overall volume proportions relatively [-1,-1,-1]..[1,1,1] cube
    */
    struct VolumeScaling
    {
        // cppcheck-suppress unusedStructMember
        float W;    //!< width  scale
        // cppcheck-suppress unusedStructMember
        float H;    //!< height scale
        // cppcheck-suppress unusedStructMember
        float D;    //!< depth  scale
    };

    struct VolumeInfo
    {
        GLuint                  volume;     //!< 3D texture ID
        GLuint                  preint;     //!< preintegration table texture
        VolumeScaling           volScaling; //!< Proportions of volume
        VolumeScaling           voxelSize;  //!< Relative volume size (0..1]
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

        const std::string&   getFileName()      const { return _filename;    }
              uint32_t       getResolution()    const { return _resolution;  }
        const VolumeScaling& getVolumeScaling() const { return _volScaling;  }

        void glewSetContext( const GLEWContext* context )
            { _glewContext = context; }

        const GLEWContext* glewGetContext() const { return _glewContext; }

    protected:

        bool _createVolumeTexture(    GLuint&                  volume,
                                      DataInTextureDimensions& TD,
                                const eq::Range&               range );

    private:
        bool _lFailed( char* msg )
            { LBERROR << msg << std::endl; return false; }

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
        uint32_t     _tW;               //!< volume texture width
        uint32_t     _tH;               //!< volume texture height
        uint32_t     _tD;               //!< volume texture depth
        uint32_t     _resolution;       //!< max( _w, _h, _d ) of a model

        VolumeScaling _volScaling;      //!< Proportions of volume

        std::vector< uint8_t >  _TF;    //!< Transfer function

        bool _hasDerivatives;           //!< true if raw+der used

        const GLEWContext*   _glewContext;    //!< OpenGL function table
    };

}

#endif // EVOLVE_RAW_VOL_MODEL_H
