
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RAM_ASYNC_LOADER_H
#define MASS_VOL__RAM_ASYNC_LOADER_H

#include <lunchbox/thread.h>

#include "ramCommands.h"

#include <boost/shared_ptr.hpp>

namespace massVolVis
{

class RAMPool;
class DataHDDIO;

typedef boost::shared_ptr<      DataHDDIO>      DataHDDIOSPtr;

/**
 *  Asynchronous fetching thread that loads block data to RAM.
 */
class RAMAsyncLoader : public lunchbox::Thread
{
public:
    RAMAsyncLoader( RAMLoadRequestQueueVec* inQueues, RAMPool* ramPool, DataHDDIOSPtr dataIO );
    ~RAMAsyncLoader();

    virtual void run();

    void postCommand( const RAMCommand& command ) {        _commands.push(   command ); }
    bool tryReadResond(     RAMRespond& respond ) { return _responds.tryPop( respond ); }
    RAMRespond readRespond()                      { return _responds.pop();             }

private:
    RAMLoadRequestQueueVec* _inQueues;      //!< loading requests form multiple channels (gpuAsuncLoaders)
    RAMCommandQueue         _commands;      //!< command communication link (in)
    RAMRespondQueue         _responds;      //!< command communication link (out)
    RAMPool*                _ramPool;       //!< main ram memory storage
    DataHDDIOSPtr           _dataIO;        //!< "block data" is loaded through this link
};

}

#endif //MASS_VOL__RAM_ASYNC_LOADER_H
