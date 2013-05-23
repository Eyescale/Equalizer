
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#ifndef MASS_VOL__RAM_POOL_H
#define MASS_VOL__RAM_POOL_H


#include "ramDataElement.h"

#include "ramCommands.h"
#include "timeStamp.h"

#include <msv/tree/nodeId.h>

#include <lunchbox/hash.h>
#include <lunchbox/lock.h>

#include <boost/shared_ptr.hpp>

namespace massVolVis
{

class DataHDDIO;
class RAMAsyncLoader;
class TimeStamper;

typedef boost::shared_ptr<      DataHDDIO>      DataHDDIOSPtr;
typedef boost::shared_ptr<const DataHDDIO> constDataHDDIOSPtr;

/**
 *  Holds data blocks in the RAM. 
 *
 *  Hash map is used to check where the data in actual storage is,
 *  and a simple vector array is then used to store values (to avoid 
 *  memory reallocation).
 *
 *  Readers should be locked/stopped during setDataInput function 
 *  call (i.e. getData and clearQueue shouldn't be called).
 */
class RAMPool
{
public:
    RAMPool();
    ~RAMPool();

    /**
     *  This is a part of initialization. Only after calling this function 
     *  data fetching would be active. (cacheSize is in MB)
     */
    void startDataLoading( DataHDDIOSPtr dataIO, const uint32_t cacheSize, const uint32_t version );

    /**
     *  Stops all data loadings.
     */
    void stopDataLoading();

    /**
     *  This function is supposed to be called from gpuAsyncLoader.
     *
     *  - Returns data pointer to the data block (0 if data is not avaliable).
     *  - When data is in the RAM it locks the data from removing
     *  (version parameter should match the one from setDataInput)
     *  - reloading is true if data was requested by the GPU for reloading
     */
    const RAMDataElement* getData( const NodeId nodeId, const uint32_t version, bool reloading );

    /**
     *  This function is supposed to be called from gpuAsyncLoader.
     *
     *  - Assigns async data fetching from HDD.
     *  - Connected pipes have separate queues, pipes are destinguished by pipePtr.
     */
    void requestData( const RAMLoadRequestVec& requests, const void* pipePtr, const uint32_t version );

    /**
     *  - Releases nodeId block, meaning it is not on the GPU anymore
     *  - If no other GPUs are using the block it is recieves current timeStamp
     */
    void releaseData( const NodeIdVec& nodeIds, const uint32_t version );

    /**
     *  This function is supposed to be called from gpuAsyncLoader.
     *
     *  - task queue for a specific pipe can be cleared using this function.
     */
    void clearQueue( const void* pipePtr );


    constDataHDDIOSPtr getDataHDDIO() const { return _dataIO; }
private:
    friend class RAMAsyncLoader; // should use only _getFreeElementAndIndex, _updateElement and _hasData

    /**
     *  This function is supposed to be called from ramAsyncLoader.
     *
     *  Returns least reacently used data element, and its index.
     */
    RAMDataElement& _getFreeElementAndIndex( size_t& index );

    /**
     *  This function is supposed to be called from ramAsyncLoader.
     *
     *  Updates node info and DataIndexHash
     */
    void _updateElement( const size_t index, const NodeId nodeId );

    /**
     *  This function is supposed to be called from ramAsyncLoader.
     *
     *  Checks if loading of requested data is necessary.
     *  Locks NodeId in the _DIHash so that it will not be loaded 
     *  multiple times by different loaders.
     */
    bool _shouldDataBeLoaded( const NodeId nodeId );


    inline bool _isRunning() const { return _running; }

    void _startRAMAsyncLoader();
    void _stopRAMAsyncLoader();

    size_t _registerNewPipe( const void* pipePtr );

    typedef stde::hash_map<       NodeId, uint32_t >  DataIndexHash;
    typedef stde::hash_map< const void* , size_t   >  PipesIndexHash;

    std::vector<RAMDataElement>  _DS;       // Data Storage
    lunchbox::Lock               _DSLock;   // Update last access time

    DataIndexHash                _DIHash;      // Loaded data Id's and their indexes in _DS
    lunchbox::Lock               _DIHashLock;

    RAMLoadRequestQueueVec       _ramLoaderQueues; // separate queue for each GPU

    PipesIndexHash               _pipesHash;    // tracking of connected pipes (_ramLoaderQueues)
    lunchbox::Lock               _pipesHashLock;

    typedef boost::shared_ptr< RAMAsyncLoader > RAMAsyncLoaderSPtr;

    std::vector<RAMAsyncLoaderSPtr> _loaders;
    DataHDDIOSPtr                   _dataIO;
    bool                            _running;

    uint32_t                        _currentVersion; // current data version

    boost::shared_ptr<TimeStamp>     _timeStampPtr;
    const std::auto_ptr<TimeStamper> _timeStamperPtr;
};

}

#endif //MASS_VOL__RAM_POOL_H
