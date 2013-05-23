
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RAM_COMMANDS_H
#define MASS_VOL__RAM_COMMANDS_H

#include <lunchbox/mtQueue.h>

#include <msv/tree/nodeId.h>


namespace massVolVis
{

/**
 *  Commands to RAM loader
 */
struct RAMCommand
{
    enum Type
    {
        PAUSE,  // ram Pool -> ram Async
        RESUME, // ram Pool -> ram Async
        EXIT,   // ram Pool -> ram Async
        NONE
    } type;

    RAMCommand( Type type_ = NONE )
        : type( type_ ){}
};

/**
 *  Commands from RAM loader
 */
struct RAMRespond
{
    enum Type
    {
        PAUSED,  // ram Async -> ram Pool
        NONE
    } type;

    RAMRespond( Type type_ = NONE )
        : type( type_ ){}
};

struct RAMLoadRequest
{
    NodeId   nodeId;

    RAMLoadRequest( NodeId nodeId_ = 0 )
        : nodeId( nodeId_ )
    {}
};

//TODO: check if shared pointers required! (if used in containers inapropriately)
typedef lunchbox::MTQueue<RAMCommand>     RAMCommandQueue;
typedef lunchbox::MTQueue<RAMRespond>     RAMRespondQueue;
typedef lunchbox::MTQueue<RAMLoadRequest> RAMLoadRequestQueue;

//TODO: check if shared pointers required!
typedef std::vector<RAMLoadRequestQueue>  RAMLoadRequestQueueVec;

typedef std::vector<RAMLoadRequest>       RAMLoadRequestVec;

} //namespace massVolVis

#endif //MASS_VOL__RAM_COMMANDS_H
