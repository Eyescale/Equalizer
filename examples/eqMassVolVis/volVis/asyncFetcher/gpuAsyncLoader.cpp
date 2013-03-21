
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012,      David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#include "gpuAsyncLoader.h"

#include "ramPool.h"
#include "ramDataElement.h"

#include "../EQ/window.h"
#include "../EQ/pipe.h"
#include "../EQ/node.h"

#include "compression/compression.h"

#include <eq/util/pixelBufferObject.h>
#include <msv/util/hlp.h>
#include <msv/util/testing.h>

#include <msv/tree/volumeTreeBase.h>
#include <ctime>

#include <msv/util/statLogger.h>
#include <msv/util/str.h>

#include <../libs/msv/IO/dataHDDIO.h>

namespace massVolVis
{

GPUAsyncLoader::GPUAsyncLoader( Window* wnd )
    : GPUAsyncLoaderBase( wnd )
    , _bytesNum(            0 )
    , _uncompressedBS(      0 )
    , _compressedBS(        0 )
    , _bytesNumNew(         0 )
    , _storageTexture3D(    0 )
    , _byteFormat3D(        0 )
    , _dataVersion(         0 )
{
    LBASSERT( wnd );
}


void GPUAsyncLoader::initialize( RAMPoolSPtr ramPool, constGPUCacheIndexSPtr cacheIndex, const byte bytesNum )
{
    _ramPool        = ramPool;
    _cacheIndexNew  = cacheIndex;
    _bytesNumNew    = bytesNum;
}


GPUAsyncLoader::~GPUAsyncLoader()
{
    postCommand( GPUCommand::EXIT );
    join();
    cleanup();
}


void GPUAsyncLoader::cleanup()
{
    _decompressor.reset();

    if( _storageTexture3D )
    {
        EQ_GL_CALL( glDeleteTextures( 1, &_storageTexture3D ));
        _storageTexture3D = 0;
    }

    if( _pbo )
        _pbo->destroy();
}


void GPUAsyncLoader::releaseRAMData( const NodeIdVec& nodeIds )
{
    if( _ramPool )
        _ramPool->releaseData( nodeIds, _dataVersion );
}


void GPUAsyncLoader::clearRAMLoadRequests()
{
    if( _ramPool )
        _ramPool->clearQueue( this );
}


void GPUAsyncLoader::postLoadRequestVec( const GPULoadRequestVec& requests )
{
    if( !_ramPool )
    {
        LBERROR << "Can't push RAM requests - RAM Pool in not initialized" << std::endl;
        return;
    }

    RAMLoadRequestVec ramRequests;
    ramRequests.reserve( requests.size() );

    for( size_t i = 0; i < requests.size(); ++i )
        ramRequests.push_back( RAMLoadRequest( requests[i].nodeId ) );

    _ramPool->requestData( ramRequests, this, _dataVersion );
    _loadRequests.push( requests );
}


void GPUAsyncLoader::_update3DTexture()
{
    if( !_storageTexture3D )
    {
        LBERROR << "Can't update 3D texture size - 3D texture is not initialized" << std::endl;
        return;
    }

    if( _bytesNumNew != 1 && _bytesNumNew != 2  )
    {
        LBERROR << "Can't update 3D texture size - only 1 or 2 bytes per value is currently allowed" << std::endl;
        return;
    }

    if( !_cacheIndexNew )
    {
        LBERROR << "GPUAsyncLoader is not initialized with _cacheIndexNew properly" << std::endl;
        return;
    }

    if(  _cacheIndex &&
        *_cacheIndex == *_cacheIndexNew &&
         _bytesNum   ==  _bytesNumNew )
        return;

    _cacheIndex = _cacheIndexNew;
    _bytesNum   = _bytesNumNew;

    const uint32_t blockSize = hlpFuncs::cubed( _cacheIndex->getBlockDim() );
    _compressedBS   = _ramPool->getDataHDDIO()->getMaxBlockSize();
    _uncompressedBS = blockSize*_bytesNum;
    LBWARN << "Compressed BS: " << _compressedBS << " Uncompressed BS: " << _uncompressedBS << std::endl;
    LBASSERT( _compressedBS <= _uncompressedBS );

    EQ_GL_CALL( glBindTexture( GL_TEXTURE_3D, _storageTexture3D ));

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );

    const Vec3_ui32 cacheDim = _cacheIndex->totalCacheTextureDim();
    LBWARN << "trying to create 3D texture of the size: " << cacheDim
           << ((_bytesNum==2)?" x2 bytes":" x1 byte") << std::endl;

    GLint internalFormat3D = (_bytesNum == 2) ? GL_ALPHA16        : GL_ALPHA;
    _byteFormat3D          = (_bytesNum == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
    EQ_GL_CALL( glTexImage3D(   GL_TEXTURE_3D,
                                0, internalFormat3D, cacheDim.w, cacheDim.h, cacheDim.d,
                                0, GL_ALPHA, _byteFormat3D, 0 ));

    // init volume tree from window
    Node* pipe = dynamic_cast<Node*>(getWindow()->getNode());
    LBASSERT( pipe );
    _volumeTree = pipe->getVolumeTree();
    LBASSERT( _volumeTree );

    _pbo = PboSPtr( new eq::util::PixelBufferObject( glewGetContext(), false ));

    if( !_pbo->setup( _uncompressedBS, GL_WRITE_ONLY_ARB ))
        LBERROR << "PBO initialization failed: " << _pbo->getError() << std::endl;

    // check that PBO has proper size
    _pbo->bind();
    int pboSize = 0;
    glGetBufferParameteriv( GL_PIXEL_UNPACK_BUFFER, GL_BUFFER_SIZE, (GLint*)&pboSize ); 
    if( (uint32_t)pboSize != _uncompressedBS )
        LBERROR << "PBO allocation failed" << std::endl;
    _pbo->unbind();

    _decompressor = Decompressors::select( _ramPool->getDataHDDIO(), _volumeTree, _pbo, _cacheIndex->capacity() );

    EQ_GL_CALL( glFinish( ));
}


bool GPUAsyncLoader::canRedecompress() const
{
    return false;
}


/**
 *  Function for creating and holding of shared context.
 *  Generation and uploading of new textures over some period with sleep time.
 */
void GPUAsyncLoader::runLocal()
{
    LBASSERT( !_storageTexture3D );
    EQ_GL_CALL( glGenTextures( 1, &_storageTexture3D ));

    // confirm successfull loading
    _loadResponds.push( GPULoadRespond( GPULoadRequest(), GPULoadStatus::INITIALIZED ));

    // start fresh in paused state
    postCommand( GPUCommand::PAUSE );

    std::string name = std::string( "GPU_Loader " ).append( strUtil::toString<>(this) );
    util::EventLogger* events = util::StatLogger::instance().createLogger( name );
    LBASSERT( events );
    uint64_t blocksLoaded   = 0;
    uint64_t blocksReloaded = 0;

    LBINFO << "async GPU fetcher initialized: " << getWindow() << std::endl;
    LBINFO << "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+" << std::endl;


    lunchbox::Clock clock;
    while( true )
    {
        // try to read new loading request
        bool sleepWait = true;

        GPULoadRequest loadRequestTmp;

        if( _loadRequests.getFront( loadRequestTmp ))
        {
//            LBWARN << "GPUAsyncLoader: trying to load request" << std::endl;
            sleepWait = false;
            _loadResponds.push( GPULoadRespond( loadRequestTmp, GPULoadStatus::STARTED ));

            GPULoadRequest loadRequest;

            bool loadingFailed = true;
            if( _decompressor && _ramPool &&
                _loadRequests.tryPop( loadRequest ) &&
                loadRequest == loadRequestTmp )
            {
                // load / request new texture
                const RAMDataElement* dataEl = _ramPool->getData( loadRequest.nodeId, _dataVersion, loadRequest.reload );

                bool sizePassed = true;
                if( dataEl && dataEl->size() < _compressedBS )
                {
                    LBERROR << "size of the data returned by RAM Pool is smaller than required by GPU" << std::endl;
                    sizePassed = false;
                }

                if( dataEl && sizePassed )
                {
                    clock.reset();

                    // LBWARN << "GPUAsyncLoader: got positive feedback from RAM Pool" << std::endl;
                    const Box_i32 coords = _cacheIndex->getBlockCoordinates( loadRequest.posOnGPU );

                    if( coords != Box_i32( ))
                    {
                        if( blocksLoaded == 0 )
                            LBWARN << "Loaded blocks to GPU: " << blocksLoaded << std::endl;

//                        LBWARN << loadRequest << std::endl;
                        _decompressor->load( dataEl, loadRequest );

                        const uint32_t blockDim = _cacheIndex->getBlockDim();
                        _pbo->bind();
                        EQ_GL_CALL( glBindTexture( GL_TEXTURE_3D, _storageTexture3D ));
                        EQ_GL_CALL( glTexSubImage3D( GL_TEXTURE_3D, 0,
                                            coords.s.x, coords.s.y, coords.s.z,
                                            blockDim, blockDim, blockDim,
                                            GL_ALPHA, _byteFormat3D, NULL ));
                        _pbo->unbind();

                        EQ_GL_CALL( glFinish( ));

                        _loadResponds.push( GPULoadRespond( loadRequest, GPULoadStatus::FINISHED ));
                        loadingFailed = false;

                        const uint32_t compressedSize = _ramPool->getDataHDDIO()->getBlockSize_( loadRequest.treePos );

                        double timeD = clock.getTimed();
                        double speedC = (compressedSize                             / (1024.f * 1024.f)) / timeD;
                        double speedD = ((blockDim*blockDim*blockDim*sizeof(float)) / (1024.f * 1024.f)) / timeD;

                        if( loadRequest.reload && _decompressor->supportsFastReloading() )
                            *events << "GBR (GPU_Block_Reloaded) " << (++blocksReloaded) << " in " << timeD << " ms, at "
                                << speedC << " MB/s (" << speedD << " MB/s)" <<std::endl;
                        else
                            *events << "GBL (GPU_Block_Loaded) " << (++blocksLoaded) << " in " << timeD << " ms, at "
                                << speedC << " MB/s (" << speedD << " MB/s)" <<std::endl;

                        if(( blocksLoaded + blocksReloaded) % 100 == 0 )
                            LBWARN << "Loaded blocks to GPU: " << blocksLoaded << " Reloaded: " << blocksReloaded << std::endl;
                    }
                }
            }
            if( loadingFailed )
                _loadResponds.push( GPULoadRespond( loadRequestTmp, GPULoadStatus::FAILED ));
        }


        // check for commands
        if( _processCommands( sleepWait )) return;

        // in case there were no commands it will sleep a bit till the next check
        if( sleepWait )
        {
            lunchbox::sleep( 20 ); // time in ms
            if( _processCommands( sleepWait )) return; // check again for new commands
        }
    }
}

bool GPUAsyncLoader::_processCommands( bool& sleepWait )
{
    GPUCommand gpuCommand;
    bool   paused = false;
    while( paused || _commands.tryPop( gpuCommand ))
    {
        if( paused ) // wait for next command if was paused
            gpuCommand = _commands.pop();

        switch( gpuCommand.type )
        {
            case GPUCommand::PAUSE:
                paused = true;
                break;
            case GPUCommand::PAUSE_AND_REPORT:
                paused = true;
                _loadResponds.push( GPULoadRespond( GPULoadRequest(), GPULoadStatus::PAUSED ));
                break;

            case GPUCommand::RESUME:
                paused    = false;
                sleepWait = false;
                break;

            case GPUCommand::UPDATE:
                _update3DTexture();
                break;

            case GPUCommand::EXIT:
                LBINFO << "Exiting GPU fetcher." << std::endl;
                LBINFO << "++==++==++==++==++==++==++==++==++==++==++==++==++==++==++==++==++==++==++==" << std::endl;
                cleanup();
                return true;

            default:
                LBERROR << "Unknown command to gpuAsyncLoader: " << static_cast<uint>(gpuCommand.type) << std::endl;
        }
    }
    return false;
}


} //namespace massVolVis
