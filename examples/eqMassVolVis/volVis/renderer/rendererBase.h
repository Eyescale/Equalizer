
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RENDERER_BASE_H
#define MASS_VOL__RENDERER_BASE_H

#include <msv/types/box.h>
#include <msv/types/vec2.h>
#include <msv/types/vmmlTypes.h>
#include <msv/types/nonCopyable.h>

#include <eq/client/gl.h>


struct GLEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;


namespace massVolVis
{

class TransferFunction;
class RenderNode;
class GLSLShaders;

/**
 * Base class for single brick rendering implementations.
 */
class RendererBase : private NonCopyable
{
public:
    RendererBase();

    virtual ~RendererBase(){}

    struct FrameInitParameters
    {
        const Matrix4d& modelviewM;
        const Matrix4f& projectionMV;
        const Vec2_f&   screenSize;
        const Vec3_f&   memBlockSize;   // textureBlockSize / textureTotalSize. [0..1]
        const Vec3_f&   innerShift;     // shift within a block
        const Vec3_f&   voxelSize;      //              1.0 / textureTotalSize. [0..1]
        const Vector3f& viewVector;
        const Vector4f& taintColor;
        const int       normalsQuality;
        const bool      drawBB;
        const uint8_t   treeDepth;
    };

    virtual void frameInit( const FrameInitParameters& fInitParams ) = 0;

    virtual void renderBrick
        (
            const RenderNode&   renderNode,
            const Box_f&        drawCoords,
            const Vec3_f&       memCoords,
            const uint32_t      numberOfSlices
        ) = 0;

    virtual void frameFinish() = 0;

    virtual bool init( GLuint ){ return true; } // general initialization, called once per frame (not per brick)

    virtual void initTF( const TransferFunction& tf ) = 0;

    virtual uint32_t getTfHist() const = 0;

    virtual std::string getName() const { return std::string( "NONE" ); }

    virtual bool isFrontToBackRendering() const { return true; }

    virtual bool canDrawBB() const { return false; }

    void glewSetContext( const GLEWContext* context ) { _glewContext = context; }
    const GLEWContext* glewGetContext() { return _glewContext; }

protected:
    bool _loadShaders( const std::string &vShader, const std::string &fShader );
    GLhandleARB _getShaderProgram() const;

private:
    const std::auto_ptr<GLSLShaders> _shadersPtr;

    const GLEWContext* _glewContext;   //!< OpenGL function table
};

} //namespace massVolVis

#endif //MASS_VOL__RENDERER_BASE_H

