
/* Copyright (c) 2009-2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__GPU_ASYNC_LOADER_H
#define MASS_VOL__GPU_ASYNC_LOADER_H

#include "gpuAsyncLoaderBase.h"

#include "gpuCommands.h"
#include "gpuCacheIndex.h"

#include <eq/client/gl.h>

#include <boost/shared_ptr.hpp>

namespace eq { namespace util { class PixelBufferObject; }}

namespace massVolVis
{

class Pipe;

class Window;

class RAMPool;
class GPUCacheIndex;
class VolumeTreeBase;
class DecompressorBase;

typedef boost::shared_ptr< RAMPool > RAMPoolSPtr;

typedef boost::shared_ptr< const VolumeTreeBase > constVolumeTreeBaseSPtr;
typedef boost::shared_ptr< const GPUCacheIndex > constGPUCacheIndexSPtr;

typedef boost::shared_ptr< eq::util::PixelBufferObject > PboSPtr;

typedef boost::shared_ptr< DecompressorBase > DecompressorSPtr;

/**
 *  Asynchronous fetching thread. Requests 
 */
class GPUAsyncLoader : public GPUAsyncLoaderBase
{
public:
    GPUAsyncLoader( Window* wnd );
    virtual ~GPUAsyncLoader();

    virtual void onInit() {}
    virtual void runLocal();
    virtual void cleanup();

    void initialize( RAMPoolSPtr ramPool, constGPUCacheIndexSPtr cacheIndex, const byte bytesNum );

    GLuint getStorageTextureId() const { return _storageTexture3D; }

    void setDataVersion( const uint32_t version ) { _dataVersion = version; }

    void postCommand( const GPUCommand& command )  { _commands.push( command  ); }

    /**
     * Will ask RAM manager for all the requested data, RAM manager will ignore
     * whatever was loaded to RAM already and will proceed with new data only.
     */
    void postLoadRequestVec( const GPULoadRequestVec& requests );
    bool tryReadLoadRespond(       GPULoadRespond&    respond )  { return _loadResponds.tryPop( respond  ); }
    GPULoadRespond readLoadRespond() { return _loadResponds.pop(); }

    void clearLoadRequests( ) { _loadRequests.clear(); }
    void clearRAMLoadRequests();

    void releaseRAMData( const NodeIdVec& nodeIds );

    bool canRedecompress() const;
private:
    void _update3DTexture();

    RAMPoolSPtr      _ramPool;
    constGPUCacheIndexSPtr _cacheIndex;
    byte             _bytesNum;
    uint32_t         _uncompressedBS;   //!< Size of uncompressed data in 3D texture
    uint32_t         _compressedBS;     //!< Size of   compressed data in 3D texture

    constGPUCacheIndexSPtr _cacheIndexNew;
    byte             _bytesNumNew;

    GLuint    _storageTexture3D;
    GLint     _byteFormat3D;
    PboSPtr   _pbo; //!< PBO for async data uploading

    constVolumeTreeBaseSPtr   _volumeTree;

    DecompressorSPtr _decompressor;

    /**
     * @return true if async fetcher should exit
     */
    bool _processCommands( bool& sleepWait );


    GPUCommandQueue         _commands;      //!< command communication link (in)
    GPULoadRequestQueue     _loadRequests;  //!< texture loading requests   (in)
    GPULoadRespondQueue     _loadResponds;  //!< texture loading responds   (out)

    uint32_t                _dataVersion;   //!< data version to ask from ramPool
};

typedef boost::shared_ptr< GPUAsyncLoader > GPUAsyncLoaderSPtr;

}

#endif //MASS_VOL__GPU_ASYNC_LOADER_H
