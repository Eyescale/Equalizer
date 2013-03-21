
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "ramPool.h"

#include "ramAsyncLoader.h"

#include <msv/IO/dataHDDIO.h>
#include <lunchbox/sleep.h>

#include "../util/unlocker.h"

#include "../asyncFetcher/timeStamper.h"


namespace massVolVis
{



RAMPool::RAMPool()
    :_running(      false   )
    ,_timeStampPtr(   new TimeStamp )
    ,_timeStamperPtr( new TimeStamper( _timeStampPtr ))
{
}


RAMPool::~RAMPool()
{
    _stopRAMAsyncLoader();
}


void RAMPool::startDataLoading( DataHDDIOSPtr dataIO, const uint32_t cacheSize, const uint32_t version )
{
    _stopRAMAsyncLoader();

    _timeStamperPtr->reset();
    _currentVersion = version;

    _dataIO = dataIO;
    if( !_dataIO )
        return;

    _ramLoaderQueues.reserve( 16 ); // maximum 16 GPUs per node

    // compute how many blocks we can store and init Data Storage
    const uint32_t blockSize = _dataIO->getBlockSize_();
    const uint32_t numberOfElements = (static_cast<uint64_t>(cacheSize) * 1024 * 1024) / blockSize;

    _DS.reserve( numberOfElements );
    uint64_t totalSize = 0;
    for( uint32_t i = 0; i < numberOfElements; ++i )
    {
        _DS.push_back( RAMDataElement( blockSize ));
        RAMDataElement& dl = _DS.back();
        dl.allocate();
        memset( dl.data(), 0, dl.size( ));
        totalSize += dl.size( );
    }

    _startRAMAsyncLoader();

    LBWARN << "RAM pool: Allocated memory cache of " << cacheSize << "MB for " << numberOfElements << " elements" << std::endl;
    LBWARN << "memsetsized: " << totalSize << std::endl;

    _running = true;
}


void RAMPool::stopDataLoading()
{
    startDataLoading( DataHDDIOSPtr(), 0, _currentVersion );
}


void RAMPool::_startRAMAsyncLoader()
{
    LBASSERT( !_isRunning() );
    LBASSERT( _loaders.size() == 0 );

    // how many loaders should there be
    for( size_t i = 0; i < 1; ++i )
    {
        RAMAsyncLoaderSPtr loader = RAMAsyncLoaderSPtr( new RAMAsyncLoader( &_ramLoaderQueues, this, _dataIO ));
        loader->start();
        _loaders.push_back( loader );
    }
}


void RAMPool::_stopRAMAsyncLoader()
{
    if( _loaders.size() == 0 )
    {
        LBWARN << "There are no running loaders" << std::endl;
        _running = false;
        return;
    }

    // stop all loaders
    _loaders.clear();
    _running = false;

    // theoretically at this point there should be no reading / writing porecces running
    Unlocker unlockerDIHash( _DIHashLock    );
    Unlocker unlockerDS(     _DSLock        );
    Unlocker unlockerPipes(  _pipesHashLock );

    _DIHash.clear();
    _DS.clear();
    _pipesHash.clear();

    for( size_t i = 0; i < _ramLoaderQueues.size(); ++i )
        _ramLoaderQueues[ i ].clear();
    _ramLoaderQueues.clear();

    LBWARN << "RAM async loader is stopped" << std::endl;
}


RAMDataElement& RAMPool::_getFreeElementAndIndex( size_t& index )
{
    LBASSERT( _isRunning() );

    // find least recently used element
    size_t    minPos;
    TimeStamp minLastAccess;

    bool success = false;
    size_t startPos = 1; // first valid index
    LBASSERT( _DS.size() > startPos+1 );
    do
    {
        while( _DS[ startPos ].isReadLocked() )
        {
            startPos++;
            if( startPos >= _DS.size() )
                startPos = 1;
        }
        minPos = startPos; // zero minPos value is reserved
        minLastAccess = _DS[ minPos ].lastAccess();

        for( size_t i = startPos; i < _DS.size(); ++i )
        {
            if( _DS[ i ].isReadLocked() )
                continue;

            const TimeStamp currentAccess = _DS[ i ].lastAccess();
            if( currentAccess < minLastAccess )
            {
                minLastAccess = currentAccess;
                minPos        = i;
            }
            if( _DS[ i ].lastAccess().isNull() )
                break;
        }
        startPos = minPos + 1;
        if( startPos >= _DS.size() )
            startPos = 1;

        // check if our match was modified, restart if that happend
        _DSLock.set();
        if( _DS[ minPos ].isReadFree() &&
            _DS[ minPos ].isWriteFree() )
            success = true; // exit loop; unlock _DS later
        else
            _DSLock.unset();

    }while( !success );

    RAMDataElement& dataElement = _DS[ minPos ];
    const NodeId oldNodeId = dataElement.getNodeId();
    dataElement.setNodeId( 0 );   // prevent data read
    dataElement.writeLock(); // prevent other writers from accessing the same memory
    _DSLock.unset();

    // clean cache and prepare the element for data loading
    _DIHashLock.set();
    if( oldNodeId != 0 )
    {
        DataIndexHash::iterator dataIterator = _DIHash.find( oldNodeId );
        LBASSERT( dataIterator != _DIHash.end() && dataIterator->second != 0 );

        _DIHash.erase( dataIterator );
    }
    _DIHashLock.unset();

    dataElement.allocate();
    LBASSERT( dataElement.data( ));

    index = minPos;

    return dataElement;
}


void RAMPool::_updateElement( const size_t index, const NodeId nodeId )
{
    LBASSERT( _isRunning( ));

    LBASSERT( index < _DS.size( ));

    _DSLock.set();
    LBASSERT( _DS[ index ].getNodeId() == 0 );
    _DS[ index ].setNodeId(     nodeId   );
    _DS[ index ].writeUnlock( *_timeStampPtr );
    _DSLock.unset();

    _DIHashLock.set();
#ifndef NDEBUG
    DataIndexHash::const_iterator dataIterator = _DIHash.find( nodeId );
    LBASSERT( dataIterator != _DIHash.end( ) && dataIterator->second == 0 );
#endif
    _DIHash[ nodeId ] = index;
    _DIHashLock.unset();
}


bool RAMPool::_shouldDataBeLoaded( const NodeId nodeId )
{
    Unlocker unlockerDIHash( _DIHashLock );

    DataIndexHash::const_iterator dataIterator = _DIHash.find( nodeId );
    if( dataIterator == _DIHash.end() ) // data was not loaded
    {
        _DIHash[ nodeId ] = 0; // mark data as being loaded
        return true;
    }
    //else don't load data again

    return false;
}


void RAMPool::releaseData( const NodeIdVec& nodeIds, const uint32_t version )
{
    if( version != _currentVersion )
        return;

    if( !_isRunning( ))
    {
        LBWARN << "Can't release data since loader is not started." << std::endl;
        return;
    }

    if( nodeIds.size() == 0 )
    {
        LBWARN << "Nothing to release." << std::endl;
        return;
    }

    std::vector< size_t > positions;
    positions.reserve( nodeIds.size() );

    // Check if data is present in the RAM
    _DIHashLock.set();
    for( size_t i = 0; i < nodeIds.size(); ++i )
    {
        DataIndexHash::const_iterator dataIterator = _DIHash.find( nodeIds[i] );
        if( dataIterator != _DIHash.end( ))
        {
            positions.push_back( dataIterator->second );
        }else
        {
            LBERROR << "Can't release data because it was not in the RAM!" << std::endl;
            positions.push_back( 0 );
        }
    }
    _DIHashLock.unset();

    LBASSERT( positions.size() == nodeIds.size() );

    _DSLock.set();
    for( size_t i = 0; i < nodeIds.size(); ++i )
    {
        const size_t position = positions[i];
        if( position == 0 )
            continue;

        RAMDataElement& dataElement = _DS[ position ];
        if( dataElement.getNodeId() != nodeIds[i] || !dataElement.isReadLocked() )
        {
            LBERROR << "Can't release data because it is corrupt" << std::endl;
            continue;
        }

        dataElement.readUnlock();
        dataElement.setLastAccess( *_timeStampPtr );
    }
    _DSLock.unset();
}


const RAMDataElement* RAMPool::getData( const NodeId nodeId, const uint32_t version, bool reloading )
{
    if( version != _currentVersion )
        return 0;

    if( !_isRunning( ))
    {
        LBWARN << "Can't retreave data since loader is not started." << std::endl;
        return 0;
    }

    // Check if data already exists in the memory
    _DIHashLock.set();
    DataIndexHash::const_iterator dataIterator = _DIHash.find( nodeId );

    if( dataIterator == _DIHash.end( )) // data is not loaded to RAM
    {
        _DIHashLock.unset();
        return 0;
    }
    // else check if data was not overriden or being loaded

    const size_t dataIndex = dataIterator->second;
    _DIHashLock.unset();

    if( dataIndex == 0 ) // requested data being loaded
        return 0;

    _DSLock.set();
    RAMDataElement* dataElement = &_DS[ dataIndex ];
    if( dataElement->getNodeId() != nodeId ) // data was overriden
    {
        _DSLock.unset();
        return 0;
    }
    // else data is on GPU

    if( !reloading )
        dataElement->readLock(); // prevent writers from overwriting
    dataElement->setLastAccess( *_timeStampPtr );
    _DSLock.unset();

    LBASSERT( dataElement->getNodeId() == nodeId );
    LBASSERT( dataElement->data() );

    return dataElement;
}


void RAMPool::requestData( const RAMLoadRequestVec& requests, const void* pipePtr, const uint32_t version )
{
    if( version != _currentVersion )
    {
        LBWARN << "Requested version doesn't match: " << version << " != " << _currentVersion << std::endl;
        return;
    }

    if( requests.size() == 0 )
    {
        LBWARN << "No data was requested" << std::endl;
        return;
    }

    // Check if we work with already known pipe, otherwise 
    // create new pipe queue.
    size_t queueIndex = 0;

    _pipesHashLock.set();
    PipesIndexHash::const_iterator pipeIterator = _pipesHash.find( pipePtr );
    if( pipeIterator != _pipesHash.end( ))
    {
        queueIndex = pipeIterator->second;
    }else
    {   // add new pipe
        queueIndex = _registerNewPipe( pipePtr );
        LBINFO << "Memory pool: Registering new pipe: " << pipePtr << std::endl;
    }
    _pipesHashLock.unset();

    _ramLoaderQueues[ queueIndex ].push( requests );
}


size_t RAMPool::_registerNewPipe( const void* pipePtr )
{
    // pause all loaders
    for( size_t i = 0; i < _loaders.size(); ++i )
    {
        RAMAsyncLoaderSPtr loader = _loaders[ i ];
        LBASSERT( loader );
        loader->postCommand( RAMCommand::PAUSE );
        if( loader->readRespond().type != RAMRespond::PAUSED )
        {
            LBERROR << "Loader was supposed to reply with 'PAUSED'" << std::endl;
        }
    }

    // add new pipe
    _ramLoaderQueues.push_back( RAMLoadRequestQueue() );
    _pipesHash[ pipePtr ] = _ramLoaderQueues.size()-1;

    // resume all loaders
    for( size_t i = 0; i < _loaders.size(); ++i )
    {
        RAMAsyncLoaderSPtr loader = _loaders[ i ];
        LBASSERT( loader );
        loader->postCommand( RAMCommand::RESUME );
    }

    return _ramLoaderQueues.size()-1;
}


void RAMPool::clearQueue( const void* pipePtr )
{
    if( !_isRunning( ))
    {
        LBWARN << "Can't clear queue since loader is not started." << std::endl;
        return;
    }

    size_t queueIndex = 0;
    size_t success    = true;

    _pipesHashLock.set();
    PipesIndexHash::const_iterator pipeIterator = _pipesHash.find( pipePtr );
    if( pipeIterator != _pipesHash.end( ))
    {
        queueIndex = pipeIterator->second;
    }
    else
    {
        success = false;
    }
    _pipesHashLock.unset();

    if( success )
        _ramLoaderQueues[ queueIndex ].clear();
    //else no such pipe
}


}

