
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RENDERER_SLICE_H
#define MASS_VOL__RENDERER_SLICE_H

#include "../rendererBase.h"

#include "../glslShaders.h"

namespace massVolVis
{

class SliceClipper;

/**
 * Texture-slicing rendering algorithm.
 */
class RendererSlice : public RendererBase
{
public:
    RendererSlice();

    virtual ~RendererSlice();

    virtual std::string getName() const { return std::string( "Texture slice rendering (Makhinya)" ); };

    virtual bool init( GLuint texture3D );

    virtual void initTF( const TransferFunction& tf );
    virtual uint32_t getTfHist() const { return 0xFFFFFFFF; } // TODO: take actual TF in to account


    virtual void frameInit( const FrameInitParameters& fInitParams );

    virtual void renderBrick
        (
            const RenderNode&   renderNode,
            const Box_f&        drawCoords,
            const Vec3_f&       memCoords,
            const uint32_t      numberOfSlices
        );

    virtual void frameFinish();

    virtual bool isFrontToBackRendering() const { return false; };

    void setPrecision( const uint32_t precision ){ _precision = precision;  }
    void setOrtho(     const uint32_t ortho     ){ _ortho     = ortho;      }

private:
    void _renderSlices( const Vec3_f& memCoords, const Box_f& blockCoords );

    bool _isInitialized;

    GLuint                  _volumeName; // storage texture
    GLuint                  _preintName; // name of the 2D preintegration texture (0 if not initialized)
    std::vector<GLubyte>    _preintData; // tf preintegration lookup

    const std::auto_ptr<SliceClipper> _sliceClipperPtr;  //!< frame clipping algorithm
    uint32_t        _precision;     //!< multiplyer for number of slices

    bool            _ortho;         //!< ortogonal/perspective projection
};

} //namespace massVolVis


#endif //MASS_VOL__RENDERER_SLICE_H

