
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RENDERER_RAYCAST_2_H
#define MASS_VOL__RENDERER_RAYCAST_2_H

#include "../rendererBase.h"

#include "../glslShaders.h"

namespace eq {
namespace util {
class FrameBufferObject;
}}


namespace massVolVis
{

class NearPlaneClipper;

/**
 * Raycasting rendering algorithm.
 * (read/write to/from same FBO)
 */
class RendererRaycast2 : public RendererBase
{
public:
    RendererRaycast2();

    virtual ~RendererRaycast2();

    virtual std::string getName() const { return std::string( "Raycasting rendering 2 (Makhinya)" ); };

    virtual bool init( GLuint texture3D );

    virtual void initTF( const TransferFunction& tf );
    virtual uint32_t getTfHist() const { return _tfHist; }

    virtual void frameInit( const FrameInitParameters& fInitParams );

    virtual bool canDrawBB() const { return true; }

    virtual void renderBrick
        (
            const RenderNode&   renderNode,
            const Box_f&        drawCoords,
            const Vec3_f&       memCoords,
            const uint32_t      numberOfSlices
        );

    virtual void frameFinish();

    void setPrecision( const uint32_t precision ){ _precision = precision;  }
    void setOrtho(     const uint32_t ortho     ){ _ortho     = ortho;      }


private:
    void _drawFbFBO();

    bool _isInitialized;

    GLuint                  _volumeName; // storage texture
    GLuint                  _preintRGBA; // name of the 2D preintegration RGBA texture (0 if not initialized)
    GLuint                  _preintSDA;  // name of the 2D preintegration SDA  texture (0 if not initialized)
    std::vector<GLfloat>    _preintData; // tf preintegration lookup

    uint32_t        _precision;     //!< multiplyer for number of slices

    typedef std::auto_ptr< eq::util::FrameBufferObject > FboAutoPtr;

    FboAutoPtr _cubeFacesFboPtr;   //!< front/back faces of cube == start/end of rays
    FboAutoPtr _frameBufferFboPtr; //!< FB replacement (to be able to read from FB while rendering)

    const std::auto_ptr< NearPlaneClipper > _nearPlaneClipperPtr;

    bool     _ortho;         //!< ortogonal/perspective projection
    int      _treeDepth;
    uint32_t _tfHist;        //!< binary quantized histogram of current TF
};

} //namespace massVolVis


#endif //MASS_VOL__RENDERER_RAYCAST_2_H

