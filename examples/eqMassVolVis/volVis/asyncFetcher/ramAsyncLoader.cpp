
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */

#include "ramAsyncLoader.h"

#include "ramPool.h"

#include <msv/util/statLogger.h>
#include <msv/util/str.h>
#include <msv/IO/dataHDDIO.h>
#include <lunchbox/sleep.h>
#include <lunchbox/clock.h>

#include <ctime>

namespace massVolVis
{


RAMAsyncLoader::RAMAsyncLoader( RAMLoadRequestQueueVec* inQueues, RAMPool* ramPool, DataHDDIOSPtr dataIO )
    : lunchbox::Thread()
    , _inQueues(    inQueues    )
    , _ramPool(     ramPool     )
    , _dataIO(      dataIO      )
{
    LBASSERT( _inQueues     );
    LBASSERT( _ramPool      );
    LBASSERT( _dataIO       );
}


RAMAsyncLoader::~RAMAsyncLoader()
{
    postCommand( RAMCommand::EXIT );
    join();
}


void RAMAsyncLoader::run()
{
    std::string name = std::string( "RAM_Loader " ).append( strUtil::toString<>(this) );
    util::EventLogger*  events = util::StatLogger::instance().createLogger( name );
    LBASSERT( events );
    uint64_t  blocksLoaded = 0;  // number of loaded blocks by current RAM loader

    LBINFO << "RAM fetcher is initialized." << std::endl;

    lunchbox::Clock clock;
    while( true )
    {
        // check for data to load
        bool sleepWait = true;
        for( size_t i = 0; i < _inQueues->size(); ++i )
        {
            RAMLoadRequest loadRequest;

            if( (*_inQueues)[i].tryPop( loadRequest ))
            {
                sleepWait = false;

                if( !_ramPool->_shouldDataBeLoaded( loadRequest.nodeId ))
                    continue;

                size_t index = 0;
                RAMDataElement& dataElement = _ramPool->_getFreeElementAndIndex( index );

                LBASSERT( dataElement.size() == _dataIO->getBlockSize_() );
                LBASSERT( dataElement.data() );

                clock.reset();
                if( _dataIO->read( loadRequest.nodeId, dataElement.data() ))
                {
                    double timeD = clock.getTimed();
                    double speed = (_dataIO->getBlockSize_() / (1024.f * 1024.f)) / timeD;
//                    LBWARN << "Sucessfully read element to RAM: " << loadRequest.nodeId << std::endl;
                    _ramPool->_updateElement( index, loadRequest.nodeId );
                    *events << "RBL (RAM_Block_Loaded) " << (++blocksLoaded) << " in " << timeD << " ms, at " << speed << " MB/s" << std::endl;
                }else
                {
                    LBERROR << "Can't read block " << loadRequest.nodeId << std::endl;
                    dataElement.reset();
                }
            }
        }

        // check for commands
        RAMCommand ramCommand;
        bool   paused = false;
        while( paused || _commands.tryPop( ramCommand ))
        {
            if( paused ) // wait for next command if was paused
                ramCommand = _commands.pop();

            switch( ramCommand.type )
            {
                case RAMCommand::EXIT: 
                    LBWARN << "Exiting RAM fetcher." << std::endl;
                    return;

                case RAMCommand::PAUSE:
                    paused = true;
                    _responds.push( RAMRespond::PAUSED );
                    break;

                case RAMCommand::RESUME:
                    paused    = false;
                    sleepWait = false;
                    break;

                default:
                    LBERROR << "Unknown command to memAsyncLoader: " << static_cast<uint>(ramCommand.type) << std::endl;
            }
        }

        // in case there were no commands it will sleep a bit till the next check
// TODO: replace this with empty command to wait for new data, tryHead for command here!
        if( sleepWait )
            lunchbox::sleep( 5 ); // time in ms
    }
}

} //namespace massVolVis
