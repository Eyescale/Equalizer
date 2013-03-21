
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__MODEL_H
#define MASS_VOL__MODEL_H

#include "transferFunction.h"
#include "rendererTypes.h"

#include "../LB/renderNode.h"
#include <asyncFetcher/gpuCacheManager.h>

#include <msv/types/nonCopyable.h>

#include <boost/shared_ptr.hpp>


namespace util{ class EventLogger; }

typedef struct GLEWContextStruct GLEWContext;


namespace massVolVis
{

class BoxN_f;
typedef std::vector<BoxN_f> vecBoxN_f;

class RendererBase;
class GPUCacheManager;
class OrderEstimator;
class CameraParameters;
class VolumeTreeBase;
struct ModelRenderParameters;

typedef boost::shared_ptr< GPUCacheManager > GPUCacheManagerSPtr;

typedef std::vector< bool > LevelsResetVec;

typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;


/**
 * - selection of renderer
 * - TF management
 */
class Model : private NonCopyable
{
public:
    Model( constVolumeTreeBaseSPtr volumeTree, byte borderDim,  // for Model
           constGPUCacheIndexSPtr cacheIndex,                   // for GPUCacheManager
           RAMPoolSPtr ramPool, const byte bytesNum,            // for GPUAsyncLoader
           Window* wnd ); // for GPUAsyncLoader, OrderEstimator

    void initTF( const TransferFunction& tf );

    void enableRenderer( const RendererType type );

    void render( const ModelRenderParameters& renderParams );

    void glewSetContext( const GLEWContext* const context );

    const GLEWContext* glewGetContext() { return _glewContext; }

    const vecBoxN_f& getDataSplit( const uint32_t          n,
                                   const VolumeTreeBase&   tree,
                                   const CameraParameters& cp );

    void setDataVersion( const uint32_t version );

    const Rect_i32& getRegion() const { return _region; }

    uint32_t bricksRendered() const { return _bricksRendered; }

    bool isFrontToBackRendering() const;

    constVolumeTreeBaseSPtr getVolumeTree() const { return _volumeTree; }

private:
    bool _checkRenderer();

    void _drawBB( const Box_f& bb );

    const GLEWContext*  _glewContext;   //!< OpenGL function table

    RenderNodeVec       _renderNodes;
    NodeIdPosVec        _desiredIds;

    GPUCacheManagerSPtr _gpuChacheMager;
    constVolumeTreeBaseSPtr _volumeTree;

    const std::auto_ptr< OrderEstimator > _orderEstimatorPtr;
          std::auto_ptr< RendererBase >   _rendererPtr;

    RendererType _rType;

    Rect_i32 _region; //!< Rendered screen region during last call of render()
    uint32_t _bricksRendered; //!< number of bricks rendered during last call of render()
    float    _cameraSpin; // !< using damping for mouse motion softening

    util::EventLogger* const _events;

    double  _timeLastFrameFinish;
    int32_t _frameNumber; // for logging

    const byte _borderDim;
};

typedef boost::shared_ptr< Model > ModelSPtr;

} //namespace massVolVis

#endif //MASS_VOL__MODEL_H

