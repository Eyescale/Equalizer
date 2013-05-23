
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 *               2012,      David Steiner   <steiner@ifi.uzh.ch>
 */

#include "pipe.h"

#include "node.h"
#include "window.h"
#include "config.h"

#include "../renderer/model.h"

#include "../asyncFetcher/gpuCacheIndex.h"

#include <msv/IO/volumeFileInfo.h>

namespace massVolVis
{

Pipe::Pipe( eq::Node* parent )
    : eq::Pipe( parent )
    , _rTypeVersion(     0 )
    , _tfVersion(        0 )
    , _fileNameVersion(  0 )
    , _lastFrameNumber(  0 )
{
    LBASSERT( parent );
}


eq::WindowSystem Pipe::selectWindowSystem() const
{
    const Config*          config   = static_cast<const Config*>( getConfig( ));
    const InitData&        initData = config->getInitData();
    const eq::WindowSystem ws       = initData.getWindowSystem();

    if( ws == eq::WINDOW_SYSTEM_NONE )
        return eq::Pipe::selectWindowSystem();

    if( !supportsWindowSystem( ws ))
    {
        LBWARN << "Window system " << ws << " not supported, using default window system" << std::endl;
        return eq::Pipe::selectWindowSystem();
    }

    return ws;
}


void Pipe::_intializeModel( Window* wnd )
{
    LBINFO << "Initializing model: " << this << ", " << wnd << std::endl;

    Node* node = static_cast<Node*>( getNode( ));
    constVolumeFileInfoSPtr volInfo = node->getVolumeFileInfo();
    if( !volInfo )
    {
        LBERROR << " Volume info is not set in the pipe" << std::endl;
        return;
    }

//TODO: add correct GPU memory size and MAX texture size
//    constGPUCacheIndexSPtr gpuCacheIndex( new GPUCacheIndex( 1400*1024*1024 / volInfo->getBytesNum(), volInfo->getBlockAndBordersDim(), 1536 ));
    constGPUCacheIndexSPtr gpuCacheIndex( new GPUCacheIndex( 800*1024*1024 / volInfo->getBytesNum(), volInfo->getBlockAndBordersDim(), 1024 ));

   _model = ModelSPtr( new Model( node->getVolumeTree(), volInfo->getBorderDim(),   // for model
                                  gpuCacheIndex,                                    // for GPUCacheManager
                                  node->getRAMPool(), volInfo->getBytesNum(),       // for GPUAsyncLoader
                                  wnd ));                                           // for GPUAsyncLoader
}


void Pipe::_deintializeModel()
{
    LBINFO << "Deinitializing model: " << this << std::endl;
    _model.reset();
}


void Pipe::frameStart( const eq::uint128_t& frameId, const uint32_t frameNumber )
{
    eq::Pipe::frameStart( frameId, frameNumber );
    _frameData.sync( frameId );
}


bool Pipe::configInit( const eq::uint128_t& initId )
{
    setIAttribute( IATTR_HINT_CUDA_GL_INTEROP, eq::ON );
    if( !eq::Pipe::configInit( initId ))
        return false;

    Config*         config      = static_cast<Config*>( getConfig( ));
    const InitData& initData    = config->getInitData();
    const lunchbox::UUID  frameDataId = initData.getFrameDataId();

    const bool mapped = config->mapObject( &_frameData, frameDataId );
    LBASSERT( mapped );

    return mapped;
}


bool Pipe::configExit()
{
    _deintializeModel();

// reset versions so that it will start clean on the next config
    _fileNameVersion = 0;
    _rTypeVersion    = 0;
    _tfVersion       = 0;

    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}


void Pipe::checkRenderingParameters( Window* wnd, uint32_t frameNumber )
{
// make these checks once per frame
    if( _lastFrameNumber >= frameNumber )
        return;

    _lastFrameNumber = frameNumber;

    const Node* node = static_cast< const Node*>( getNode( ));

// enable new data if required (from node!)
    const uint32_t fileNameVersion = node->getFileNameVersion();
    if( _fileNameVersion < fileNameVersion )
    {
        LBWARN << "Setting new data" << std::endl;

        // make sure async stuff is stopped before new Model is constructed
        _deintializeModel();
        _intializeModel( wnd );

        if( !_model )
            return;

        _fileNameVersion = fileNameVersion;
        _model->setDataVersion( _fileNameVersion );
    }

    if( !_model )
        return;

    constVolumeInfoSPtr volumeInfo = node->getVolumeInfo();

// set renderer if required (from volumeInfo)
    const uint32_t rTypeVersion = volumeInfo->getRendererTypeVersion();
    if( _rTypeVersion < rTypeVersion )
    {
        LBWARN << "Setting new renderer" << std::endl;

        _model->enableRenderer( volumeInfo->getRendererType( ));

        _rTypeVersion = rTypeVersion;
    }

// set TF if required (from volumeInfo)
    const uint32_t tfVersion = volumeInfo->getTransferFunctionVersion();
    if( _tfVersion < tfVersion )
    {
        _model->initTF( volumeInfo->getTransferFunction( ));
        _tfVersion = tfVersion;
    }
}


}//namespace massVolVis

