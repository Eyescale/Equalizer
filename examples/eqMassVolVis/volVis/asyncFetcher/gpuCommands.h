
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */
// // 
#ifndef MASS_VOL__GPU_COMMANDS_H
#define MASS_VOL__GPU_COMMANDS_H

#include <lunchbox/mtQueue.h>

#include <msv/tree/nodeId.h>
#include <msv/types/types.h>


namespace massVolVis
{

/**
 *  Commands to GPU loader
 */
struct GPUCommand
{
    enum Type
    {
        PAUSE,              // model -> gpu Async
        PAUSE_AND_REPORT,   // pipe  -> gpu Async (gpu Async will send a reply)
        RESUME,             // model -> gpu Async
        UPDATE,             // pipe  -> gpu Async (update 3D cache)
        EXIT,               // pipe  -> gpu Async
        NONE
    } type;

    GPUCommand( Type type_ = NONE )
        : type( type_ ){}
};


struct GPULoadRequest
{
    NodeId   nodeId;
    uint32_t posOnGPU;
    uint32_t treePos;
    bool     reload;

    GPULoadRequest() : nodeId(0), posOnGPU(0), treePos(0), reload( false ) {}

    explicit GPULoadRequest( NodeId nodeId_, uint32_t posOnGPU_, uint32_t treePos_, bool reload_ )
        : nodeId(   nodeId_     )
        , posOnGPU( posOnGPU_   )
        , treePos(  treePos_    )
        , reload(   reload_     )
    {}

    bool operator == ( const GPULoadRequest& other ) const
    {
        return nodeId   == other.nodeId   &&
               posOnGPU == other.posOnGPU &&
               reload   == other.reload  ;
    }
};

namespace
{
std::ostream& operator<<( std::ostream& out, const GPULoadRequest& req )
{
    out << "GPULoadRequest [" << req.nodeId    << ", "
                              << req.posOnGPU  << ", "
                              << (req.reload ? "reload" : "new_data" ) << ")";
    return out;
}
}

typedef std::vector<GPULoadRequest> GPULoadRequestVec;

/**
 *  Replys from GPU loader
 */
struct GPULoadStatus
{
    enum Type
    {
        STARTED,  // gpu Async -> model     - GPU fetcher will try to load a brick data to GPU
        FINISHED, // gpu Async -> model     - data was sucessfully loaded to GPU
        FAILED,   // gpu Async -> model     - data was not avaliable or request was canceled
        PAUSED,   // gpu Async -> pipe      - loader is paused (as a reply to PAUSE_AND_REPORT)
        INITIALIZED, // gpu Async -> pipe    - intialization is finished
        NONE
    } value;

    GPULoadStatus( Type value_ = NONE )
        : value( value_ ){}
};


struct GPULoadRespond : GPULoadRequest
{
    GPULoadStatus status;

    GPULoadRespond(){}

    GPULoadRespond( const GPULoadRequest& loadRequest, GPULoadStatus status_ )
        : GPULoadRequest( loadRequest )
        , status(         status_     )
    {}
};


typedef lunchbox::MTQueue<GPUCommand>     GPUCommandQueue;
typedef lunchbox::MTQueue<GPULoadRequest> GPULoadRequestQueue;
typedef lunchbox::MTQueue<GPULoadRespond> GPULoadRespondQueue;

} //namespace massVolVis

#endif //MASS_VOL__GPU_COMMANDS_H
