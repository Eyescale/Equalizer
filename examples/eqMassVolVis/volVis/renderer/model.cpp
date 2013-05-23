//std12

#include "model.h"

#include "raycasting/rendererRaycast.h"
#include "raycasting/rendererRaycast2.h"
#include "slice/rendererSlice.h"
#include "rendererBase.h"

#include "../asyncFetcher/gpuCacheManager.h"
#include "../EQ/channel.h"

#include "../LB/orderEstimator.h"
#include "../LB/cameraParameters.h"
#include <msv/tree/volumeTreeBase.h>

#include <msv/util/statLogger.h>
#include <msv/util/str.h>

#include "modelRenderParameters.h"

namespace massVolVis
{



namespace
{
void _drawRectangles( const Channel* channel, const RenderNodeVec& rNodes );

util::EventLogger* _getEventLogger( void* p )
{
    return util::StatLogger::instance().createLogger(
                std::string( "Model " ).append( strUtil::toString<>( p ) ) );
}

}


Model::Model( constVolumeTreeBaseSPtr volumeTree, byte borderDim,
              constGPUCacheIndexSPtr cacheIndex, RAMPoolSPtr ramPool, const byte bytesNum,
              Window* wnd )
    : _gpuChacheMager( new GPUCacheManager( cacheIndex, ramPool, bytesNum, wnd ))
    , _volumeTree( volumeTree )
    , _orderEstimatorPtr( new OrderEstimator())
    , _rType( NONE )
    , _bricksRendered( 0 )
    , _cameraSpin( 0 )
    , _events( _getEventLogger( this ))
    , _timeLastFrameFinish( 0 )
    , _frameNumber( 0 )
    , _borderDim( borderDim )
{
    LBASSERT( volumeTree );
    LBASSERT( _events );
}


void Model::setDataVersion( const uint32_t version )
{
    _gpuChacheMager->setDataVersion( version );
}


bool Model::_checkRenderer()
{
    if( !_rendererPtr.get() )
    {
        LBERROR << "Renderer is not set" << std::endl;
        return false;
    }
    return true;
}


void Model::glewSetContext( const GLEWContext* context )
{
    _glewContext = context;

    if( _checkRenderer( ))
        _rendererPtr->glewSetContext( context );
}


void Model::initTF( const TransferFunction& tf )
{
    if( _checkRenderer( ))
        _rendererPtr->initTF( tf );
}


bool Model::isFrontToBackRendering() const
{
    return _rendererPtr.get() ? _rendererPtr->isFrontToBackRendering() : false;
}


void Model::enableRenderer( const RendererType type )
{
    if( _rType == type )
        return;

    switch( type )
    {
        case SLICE:
            _rendererPtr = std::auto_ptr<RendererBase>( new RendererSlice() );
            break;

        case RAYCAST:
//            _renderer = std::auto_ptr<RendererBase>( new RendererRaycast() );
            _rendererPtr = std::auto_ptr<RendererBase>( new RendererRaycast2() );
            break;

        default:
            _rendererPtr.reset();
            LBERROR << "Wrong renderer type!" << int(type) << std::endl;
            return;
    }

    _rType = type;

    LBWARN << "New renderer is set to: " << _rendererPtr->getName().c_str() << std::endl;
}


const vecBoxN_f& Model::getDataSplit( const uint32_t          n,
                                      const VolumeTreeBase&   tree,
                                      const CameraParameters& cp )
{
    return _orderEstimatorPtr->getDataSplit( n, tree, cp );
}

namespace
{
void _adjustBudjet( uint32_t& budget, float& cameraSpin, float newCameraSpin )
{
    if( cameraSpin < newCameraSpin )
        cameraSpin = 0.5*cameraSpin + 0.5*newCameraSpin;
    else
        cameraSpin = 0.2*cameraSpin + 0.8*newCameraSpin;

    const float mins = 0.001;
    const float maxs = 0.1;
    const float Mins = 109.0;
    const float Maxs =  10.0;
    const float as = (Maxs-Mins)*(maxs*mins)/(mins-maxs);
    const float bs = Mins - as/mins;
    if( cameraSpin > 0.0000001 )
    {
        if( cameraSpin > maxs )
            budget = EQ_MIN( budget, Maxs );
        else
            budget = EQ_MIN( budget, as / cameraSpin + bs );
    }
//    std::cout << "spin: " << cameraSpin << " a: " << as << " b: " << bs << " budget: " << budget <<  std::endl;
}
}

void Model::render( const ModelRenderParameters& rParams )
{
    if( !_checkRenderer( ) || !_gpuChacheMager )
        return;

    if( !_rendererPtr->init( _gpuChacheMager->getStorageTextureId( )))
        return;

    _frameNumber++;
    *_events << "NF (New_Frame): " << _frameNumber << std::endl;
    double timeStart = _events->getTimed();

    const uint32_t gpuCapacity = _gpuChacheMager->capacity();
    if( gpuCapacity < 1 )
    {
        LBERROR << "No memory is allocated on GPU" << std::endl;
        return;
    }

    // Compute current front and render avaliable bricks
    const Vec3_f        voxelSize    = _gpuChacheMager->getVoxelScale();
    const Vec3_f        memBlockSize = _gpuChacheMager->getBlockScale();
    const Vec3_f        innerShift   = voxelSize * _borderDim;
    const int           normalsQuality  = 2;
    const RendererBase::FrameInitParameters fInitParams = { rParams.modelviewM,
                                                            rParams.projectionMV,
                                                            rParams.screenSize,
                                                            memBlockSize,
                                                            innerShift,
                                                            voxelSize,
                                                            rParams.viewVector,
                                                            rParams.taintColor,
                                                            normalsQuality,
                                                            rParams.drawBB,
                                                            _volumeTree->getDepth() };
    _rendererPtr->frameInit( fInitParams );
    CameraParameters cp( rParams.modelviewM, rParams.projectionMV, rParams.screenCenter, rParams.screenSize );

    const uint32_t blocksBudget = (gpuCapacity > 1 ? gpuCapacity - 1 : 1); // proper selection
//    const uint32_t blocksBudget = std::min( (gpuCapacity > 1 ? gpuCapacity - 1 : 1), 150u );
//    const uint32_t blocksBudget = 1;
//    const uint32_t blocksBudget = 75;

//    std::cout << "gpu cap: " << gpuCapacity << " bb: " << blocksBudget << std::endl;

    _desiredIds.resize( 0 );

    uint32_t dbSegmentsN = 1;
    if( rParams.range != eq::Range::ALL )
        dbSegmentsN = (eq::Range::ALL.getSize() + rParams.range.getSize()/2.0) /
                                                                rParams.range.getSize();
    const uint32_t segmentNumber =
        (rParams.range.start-eq::Range::ALL.start + rParams.range.getSize()/2.0 ) /
                                                                rParams.range.getSize();

    LBASSERT( segmentNumber < dbSegmentsN );

    const vecBoxN_f& bbs = _orderEstimatorPtr->getDataSplit( dbSegmentsN, *_volumeTree, cp );

    int32_t blockNum = -1;
    for( size_t i = 0; i < bbs.size(); ++i )
        if( bbs[i].pos == segmentNumber )
        {
            blockNum = i;
            break;
        }
    LBASSERT( blockNum >= 0 );
    const Box_f& dataBB = bbs[blockNum].bb;

    uint32_t budget = EQ_MIN( blocksBudget, rParams.renderBudget );

//    _adjustBudjet( budget, _cameraSpin, rParams.cameraSpin );

    _orderEstimatorPtr->compute(
        _renderNodes,
        _desiredIds,
        *_gpuChacheMager,
        *_volumeTree,       // octree
        dataBB,             // what block to render
        cp,                 // camera parameters
        budget,             // number of blocks
        _rendererPtr->getTfHist(),
        1000.f/1.f,        // ms of rendering
        rParams.maxTreeDepth,
        _rendererPtr->isFrontToBackRendering(),
        rParams.useRenderingError,
        rParams.renderingError
//        true,
//        390
     );
    const double timeBudget = _events->getTimed();
    const double timeBudgetDelta = timeBudget - timeStart;
    *_events << "BE (Budget_Estimation) " << timeBudgetDelta << " ms (" << (1000.f / timeBudgetDelta) << " FPS)" << std::endl;
    static int tIt = 0;
    static double tSum = 0.0;
    static double lastDisplayTime = 0.0;
    tSum += timeBudgetDelta;
    tIt++;
    if( tIt == 50 || timeBudget - lastDisplayTime > 2000.0 )
    {
        std::stringstream ss;
        ss << "Last ord est speed: " << tSum / tIt << " desired nodes: " << _desiredIds.size() << " rendered nodes: " << _renderNodes.size() << std::endl;

        std::cout << ss.str().c_str() << std::flush;

        tIt  = 0;
        tSum = 0;
        lastDisplayTime = timeBudget;
    }

    _gpuChacheMager->updateFront( _desiredIds );

    // reset rendered region
    _region = Rect_i32( rParams.screenCenter+rParams.screenSize, rParams.screenCenter-rParams.screenSize );

    _bricksRendered = 0;

    const double timeUpdateFront = _events->getTimed();
    const double timeUpdateFrontDelta = timeUpdateFront - timeBudget;
    *_events << "UF (Update_Front) " << timeUpdateFrontDelta << " ms (" << (1000.f / timeUpdateFrontDelta) << " FPS)" << std::endl;

    for( size_t i = 0; i < _renderNodes.size(); ++i )
    {
        const Box_f drawCoords = dataBB.intersect( _renderNodes[i].coords );
        if( !drawCoords.valid( ))
        {
            LBWARN << "Intersection of data BB and data block is empty!" << std::endl;
            continue;
        }

        const Vec3_f memCoords = _gpuChacheMager->getGPUMemoryOffset( _renderNodes[i].nodeId );
        const uint32_t numberOfSlices = std::min( _renderNodes[i].screenSize / 2, static_cast<uint32_t>( _volumeTree->getBlockSize()));
        _rendererPtr->renderBrick( _renderNodes[i], drawCoords, memCoords, numberOfSlices );
        _region.expand( _renderNodes[i].screenRect );
        ++_bricksRendered;
    }
    _rendererPtr->frameFinish();

    const double timeRender = _events->getTimed();
    const double timeRenderDelta = timeRender - timeUpdateFront;
    const double timeTotalRender = timeRender - timeStart;
    const double timeFrameDelta = timeRender - _timeLastFrameFinish;
    _timeLastFrameFinish = timeRender;

    _orderEstimatorPtr->updateRenderingStats( _renderNodes, timeFrameDelta );

    *_events << "RB  (Rendered_Bricks) " << _bricksRendered << " in " << timeRenderDelta << " ms (" << (1000.f / timeRenderDelta) << " FPS)" << std::endl;
    *_events << "RT  (Rendering_Time) "  << _bricksRendered << " in " << timeTotalRender << " ms (" << (1000.f / timeTotalRender) << " FPS)" << std::endl;
    *_events << "TFT (Total_Frame_Time) " << timeFrameDelta << " ms (" << (1000.f / timeFrameDelta) << " FPS)" << std::endl;

    if( _renderNodes.size() == 0 )
        lunchbox::sleep( 900 );

    // draw bounding boxes
    if( rParams.drawBB && !_rendererPtr->canDrawBB() )
        for( size_t i = 0; i < _renderNodes.size(); ++i )
            _drawBB( _renderNodes[i].coords );

//    _drawRectangles( rParams.channel, _renderNodes );
}


/*
//
// Uploading of all data to the GPU first, then rendering:
//
void Model::render( const eq::Matrix4f& modelviewM,
                    const eq::Matrix4f& projectionMV,
                    const eq::Vector2f& screenSize,
                    const eq::Vector3f& viewVector  )
{
    if( !_checkRenderer( ))
        return;

    if( !_rendererPtr->init( _pipe->getStorageTextureId( )))
        return;

    const uint32_t gpuCapacity = _gpuChacheMager->capacity();
    if( gpuCapacity < 1 )
    {
        LBERROR << "No memory is allocated on GPU" << std::endl;
        return;
    }

    // Compute current front and render avaliable bricks
    const Vec3_f        voxelSize     = _gpuChacheMager->getVoxelScale();
    const Vec3_f        memBlockSize = _gpuChacheMager->getBlockScale();
    const Vec3_f        innerShift = voxelSize;
    const eq::Vector4f  taintColor( 0.f, 0.f, 0.25f,0.0f );
    const int           normalsQuality  = 2;
    _rendererPtr->frameInit
    (
        modelviewM,
        memBlockSize,
        innerShift,
        voxelSize,
        viewVector,
        taintColor,
        normalsQuality
    );

    CameraParameters cp( modelviewM, projectionMV, screenSize );

    const VolumeTree* tree = _pipe->getVolumeTree();
    LBASSERT( tree );

//    const uint32_t blocksBudget = 1;
    const uint32_t blocksBudget = gpuCapacity > 1 ? gpuCapacity - 1 : 1;


    _desiredIds.resize( 0 );
    const NodeId maxId = tree->getMaxBlockId();
    for( uint32_t id = 1; id <= maxId; ++id )
    {
        _desiredIds.push_back( id );
    }
    _gpuChacheMager->updateFront( _desiredIds );

    bool all = true;
    for( uint32_t id = 1; id <= maxId; ++id )
    {
        if( !_gpuChacheMager->hasNodeOnGPU( id ))
        {
            all = false;
            break;
        }
    }
    if( all )
    {
        static bool msg1Done = false;
        if( !msg1Done )
        {
            LBWARN << "All " << maxId << " blocks are ready" << std::endl;
            msg1Done = true;
        }

        const Box_f dataBB( 0.f, 0.f, 0.f, 1.f, 1.f, 1.f );

        _orderEstimatorPtr->compute(
            _renderNodes,
            _desiredIds,
            *_gpuChacheMager,
            *tree,                          // octree
            dataBB,      // what block to render
            cp,                             // camera parameters
            blocksBudget,                   // number of blocks
            1000 );                         // ms of rendering

    _gpuChacheMager->updateFront( _desiredIds );

        static bool msg2Done = false;
        static int      count       = 0;
        static uint64_t blocksCount = 0;
        if( !msg2Done )
        {
            LBWARN << "Rendering bricks: " << _renderNodes.size() << std::endl;
            msg2Done = true;
        }

        count++;
        blocksCount += _renderNodes.size();

        if( count == 1000 )
        {
            LBWARN << "Rendering of 1000 frames done. Average block count: " << blocksCount / count << std::endl;
        }


    for( size_t i = 0; i < _renderNodes.size(); ++i )
    {
        const Box_f drawCoords = dataBB.intersect( _renderNodes[i].coords );
        if( !drawCoords.valid( ))
        {
            LBWARN << "Intersection of data BB and data block is empty!" << std::endl;
            continue;
        }

        const Vec3_f memCoords = _gpuChacheMager->getGPUMemoryOffset( _renderNodes[i].nodeId );
        const uint32_t numberOfSlices = std::min( _renderNodes[i].screenSize / 2, static_cast<uint32_t>( tree->getBlockSize()));
        _rendererPtr->renderBrick( _renderNodes[i].coords, drawCoords, memCoords, numberOfSlices );

//        LBWARN << _renderNodes[i] << " mem: " << memCoords << std::endl;
    }
    }

    _rendererPtr->frameFinish();

    if( _renderNodes.size() == 0 )
        lunchbox::sleep( 900 );

    // TODO: Disable of preloading of all blocks
    LBWARN << "new boxes: " << std::endl;
    const vecBox_f& bbs = _orderEstimatorPtr->getDataSplit( 8, *tree );
    for( size_t i = 0; i < bbs.size(); ++i )
        LBWARN << "i: " << bbs[i] << std::endl;

    // draw bounding boxes
//    for( size_t i = 0; i < _renderNodes.size(); ++i )
//        _drawBB( _renderNodes[i].coords );
}
*/

/*
void Model::render( const eq::Matrix4f& ,
                    const eq::Matrix4f& ,
                    const eq::Vector2f& ,
                    const eq::Vector3f& )
{
    if( !_checkRenderer( ))
        return;

    if( !_rendererPtr->init( _pipe->getStorageTextureId( )))
        return;

    const uint32_t gpuCapacity = _gpuChacheMager->capacity();
//    LBWARN << "gpuCapacity: " << gpuCapacity << std::endl;
    if( gpuCapacity < 1 )
    {
        LBERROR << "No memory is allocated on GPU" << std::endl;
        return;
    }

    static int phase = 0;

    NodeId startID = 1;
    switch( phase )
    {
        case 0: startID = 3000; break;
        case 1: startID = 4000; break;
        case 2: startID = 3000; break;
        case 3: startID = 4000; break;
    }

    _desiredIds.resize( 0 );
    NodeId id = startID;
    for( uint32_t i = 0; i < 1000; ++i )
    {
        _desiredIds.push_back( id++ );
    }

    bool all = true;
    id = startID;
    for( uint32_t i = 0; i < 1000; ++i )
    {
        if( !_gpuChacheMager->hasNodeOnGPU( id++ ))
        {
            all = false;
            break;
        }
    }
    if( all )
    {
        LBWARN << "Stage " << phase << " is finished" << std::endl;
        phase++;
    }

    _gpuChacheMager->updateFront( _desiredIds );

    lunchbox::sleep( 90 );
}
*/

namespace
{
void _drawRectangles( const Channel* channel, const RenderNodeVec& rNodes )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    channel->applyScreenFrustum();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );

    glColor3f( 1.0f, 1.0f, 1.0f );


    for( size_t i = 0; i < rNodes.size(); ++i )
    {
        const Rect_i32& rect = rNodes[i].screenRect;
        glBegin( GL_LINE_LOOP );
            glVertex3f( rect.s.x, rect.s.y, 0.0f );
            glVertex3f( rect.e.x, rect.s.y, 0.0f );
            glVertex3f( rect.e.x, rect.e.y, 0.0f );
            glVertex3f( rect.s.x, rect.e.y, 0.0f );
        glEnd();
    }

}
}


void Model::_drawBB( const Box_f& bb )
{
    glDisable( GL_LIGHTING );

    glEnable( GL_FOG );
    {
        const GLfloat fogColor[4] = { 1.0, 0.8, 0.8, 1.0 };

        glFogi(  GL_FOG_MODE,    GL_EXP       );
        glFogfv( GL_FOG_COLOR,   fogColor     );
        glFogf(  GL_FOG_DENSITY, 0.5          );
        glHint(  GL_FOG_HINT,    GL_DONT_CARE );
        glFogf(  GL_FOG_START,    1.0         );
        glFogf(  GL_FOG_END,     10.0         );
    }
    glClearColor( 1.0, 1.0, 1.0, 1.0 );

    glLineWidth( 1.5f );
    glColor3f( 1.f,  0.f,  0.f );
    glBegin( GL_LINE_STRIP );
        glVertex3f( bb.s.x,  bb.s.y, bb.s.z );
        glVertex3f( bb.s.x,  bb.e.y, bb.s.z );
        glVertex3f( bb.e.x,  bb.e.y, bb.s.z );
        glVertex3f( bb.e.x,  bb.s.y, bb.s.z );
        glVertex3f( bb.s.x,  bb.s.y, bb.s.z );

        glVertex3f( bb.s.x,  bb.s.y, bb.e.z );
        glVertex3f( bb.s.x,  bb.e.y, bb.e.z );
        glVertex3f( bb.e.x,  bb.e.y, bb.e.z );
        glVertex3f( bb.e.x,  bb.s.y, bb.e.z );
        glVertex3f( bb.s.x,  bb.s.y, bb.e.z );
    glEnd();
    glBegin( GL_LINES );
        glVertex3f( bb.s.x,  bb.e.y, bb.s.z );
        glVertex3f( bb.s.x,  bb.e.y, bb.e.z );

        glVertex3f( bb.e.x,  bb.e.y, bb.s.z );
        glVertex3f( bb.e.x,  bb.e.y, bb.e.z );

        glVertex3f( bb.e.x,  bb.s.y, bb.s.z );
        glVertex3f( bb.e.x,  bb.s.y, bb.e.z );
    glEnd();

    glDisable( GL_FOG      );
    glEnable(  GL_LIGHTING );
}


} //namespace massVolVis
