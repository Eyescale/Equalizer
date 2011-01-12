
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "versionedSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectPackets.h"
#include <co/base/scopedMutex.h>
#include <limits>

namespace co
{
typedef CommandFunc<VersionedSlaveCM> CmdFunc;

VersionedSlaveCM::VersionedSlaveCM( Object* object, uint32_t masterInstanceID )
        : _object( object )
        , _version( VERSION_NONE )
        , _currentIStream( 0 )
        , _masterInstanceID( masterInstanceID )
#pragma warning(push)
#pragma warning(disable: 4355)
        , _ostream( this )
#pragma warning(pop)
{
    EQASSERT( object );
    EQASSERT( object->getLocalNode( ));
    CommandQueue* q = object->getLocalNode()->getCommandThreadQueue();

    object->registerCommand( CMD_OBJECT_INSTANCE,
                             CmdFunc( this, &VersionedSlaveCM::_cmdInstance ),
                             q );
    object->registerCommand( CMD_OBJECT_DELTA,
                             CmdFunc( this, &VersionedSlaveCM::_cmdDelta ), q );
    object->registerCommand( CMD_OBJECT_COMMIT, 
                             CmdFunc( this, &VersionedSlaveCM::_cmdCommit ), q);
}

VersionedSlaveCM::~VersionedSlaveCM()
{
    while( !_queuedVersions.isEmpty( ))
        delete _queuedVersions.pop();

    EQASSERT( !_currentIStream );
    delete _currentIStream;
    _currentIStream = 0;

    _version = VERSION_NONE;
    _master = 0;
}

const NodeID& VersionedSlaveCM::getMasterNodeID() const
{ 
    if( !_master )
        return co::base::UUID::ZERO;
        
    return _master->getNodeID();
}

uint32_t VersionedSlaveCM::commitNB()
{
    LocalNodePtr localNode = _object->getLocalNode();
    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = localNode->registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint128_t VersionedSlaveCM::commitSync( const uint32_t commitID )
{
    LocalNodePtr localNode = _object->getLocalNode();
    uint128_t version = VERSION_NONE;
    localNode->waitRequest( commitID, version );
    return version;
}

uint128_t VersionedSlaveCM::sync( const uint128_t& v )
{
#if 0
    EQLOG( LOG_OBJECTS ) << "sync to v" << v << ", id " << _object->getID()
                         << "." << _object->getInstanceID() << std::endl;
#endif
    if( _version == v )
        return _version;

    const uint128_t version = ( v == VERSION_NEXT ) ? _version + 1 : v;

    if( version == VERSION_HEAD )
    {
        _syncToHead();
        return _version;
    }

    EQASSERTINFO( version.high() == 0, "Not a master version: " << version )
    EQASSERTINFO( _version <= version,
                  "can't sync to older version of object " << 
                  typeid( *_object ).name() << " " << _object->getID() <<
                  " (" << _version << ", " << version <<")" );

    while( _version < version )
    {
        ObjectDataIStream* is = _queuedVersions.pop();
        _unpackOneVersion( is );
        EQASSERTINFO( _version == is->getVersion(), "Have version " 
                      << _version << " instead of " << is->getVersion( ));
        is->reset();
        _iStreamCache.release( is );
    }

    LocalNodePtr node = _object->getLocalNode();
    if( node.isValid( ))
        node->flushCommands();

    return _version;
}

void VersionedSlaveCM::_syncToHead()
{
    if( _queuedVersions.isEmpty( ))
        return;

    ObjectDataIStream* is = 0;
    while( _queuedVersions.tryPop( is ))
    {
        EQASSERT( is );
        _unpackOneVersion( is );
        EQASSERTINFO( _version == is->getVersion(), "Have version " 
                      << _version << " instead of " << is->getVersion( ));
        is->reset();
        _iStreamCache.release( is );
    }

    LocalNodePtr localNode = _object->getLocalNode();
    if( localNode.isValid( ))
        localNode->flushCommands();
}


uint128_t VersionedSlaveCM::getHeadVersion() const
{
    ObjectDataIStream* is = 0;
    if( _queuedVersions.getBack( is ))
    {
        EQASSERT( is );
        return is->getVersion();
    }
    return _version;    
}

void VersionedSlaveCM::_unpackOneVersion( ObjectDataIStream* is )
{
    EQASSERT( is );
    EQASSERTINFO( _version == is->getVersion() - 1, "Expected version " 
                  << _version + 1 << ", got " << is->getVersion() << " for "
                  << *_object );

    if( is->hasInstanceData( ))
        _object->applyInstanceData( *is );
    else
        _object->unpack( *is );

    _version = is->getVersion();
    EQASSERT( _version != VERSION_INVALID );
    EQASSERT( _version != VERSION_NONE );
#if 0
    EQLOG( LOG_OBJECTS ) << "applied v" << _version << ", id "
                         << _object->getID() << "." << _object->getInstanceID()
                         << std::endl;
#endif

    EQASSERTINFO( is->getRemainingBufferSize()==0 && is->nRemainingBuffers()==0,
                  "Object " << typeid( *_object ).name() <<
                  " did not unpack all data" );
}


void VersionedSlaveCM::applyMapData( const uint128_t& version )
{
    while( true )
    {
        ObjectDataIStream* is = _queuedVersions.pop();
        if( is->getVersion() == version )
        {
            EQASSERTINFO( is->hasInstanceData(), *_object );

            _object->applyInstanceData( *is );
            _version = is->getVersion();
            EQASSERT( _version != VERSION_INVALID );

            EQASSERTINFO( is->getRemainingBufferSize()==0 &&
                          is->nRemainingBuffers()==0,
                          co::base::className( _object ) <<
                          " did not unpack all data, " <<
                          is->getRemainingBufferSize() << " bytes, " <<
                          is->nRemainingBuffers() << " buffer(s)" );

            is->reset();
            _iStreamCache.release( is );
#if 0
            EQLOG( LOG_OBJECTS ) << "Mapped initial data of " << _object
                                 << std::endl;
#endif
            return;
        }
        else
        {
            // Found the following case:
            // - p1, t1 calls commit
            // - p1, t2 calls mapObject
            // - p1, cmd commits new version
            // - p1, cmd subscribes object
            // - p1, rcv attaches object
            // - p1, cmd receives commit data
            // -> newly attached object recv new commit data before map data,
            //    ignore it
            EQASSERTINFO( is->getVersion() > version,
                          is->getVersion() << " <= " << version );
            is->reset();
            _iStreamCache.release( is );
        }
    }
}

void VersionedSlaveCM::addInstanceDatas(
    const ObjectDataIStreamDeque& cache, const uint128_t& startVersion )
{
    EQ_TS_THREAD( _cmdThread );
#if 0
    EQLOG( LOG_OBJECTS ) << co::base::disableFlush << "Adding data front ";
#endif

    uint128_t oldest = VERSION_NONE;
    uint128_t newest = VERSION_NONE;
    if( !_queuedVersions.isEmpty( ))
    {
        ObjectDataIStream* is = 0;

        EQCHECK( _queuedVersions.getFront( is ));
        oldest = is->getVersion();

        EQCHECK( _queuedVersions.getBack( is ));
        newest = is->getVersion();
    }

    ObjectDataIStreamDeque head;
    ObjectDataIStreams tail;

    for( ObjectDataIStreamDeque::const_iterator i = cache.begin();
         i != cache.end(); ++i )
    {
        ObjectDataIStream* stream = *i;
        const uint128_t& version = stream->getVersion();
        if( version < startVersion )
            continue;
        
        EQASSERT( stream->isReady( ));
        EQASSERT( stream->hasInstanceData( ));
        if( !stream->isReady( ))
            break;

        if( version < oldest )
            head.push_front( stream );
        else if( version > newest )
            tail.push_back( stream );
    }

    for( ObjectDataIStreamDeque::const_iterator i = head.begin();
         i != head.end(); ++i )
    {
        const ObjectDataIStream* stream = *i;
#ifndef NDEBUG
        ObjectDataIStream* debugStream = 0;
        _queuedVersions.getFront( debugStream );
        if( debugStream )
            EQASSERT( debugStream->getVersion() == stream->getVersion() + 1);
#endif
        _queuedVersions.pushFront( new ObjectDataIStream( *stream ));
#if 0
        EQLOG( LOG_OBJECTS ) << stream->getVersion() << ' ';
#endif
    }

#if 0
    EQLOG( LOG_OBJECTS ) << " back ";
#endif
    for( ObjectDataIStreams::const_iterator i = tail.begin();
         i != tail.end(); ++i )
    {
        const ObjectDataIStream* stream = *i;
#ifndef NDEBUG
        ObjectDataIStream* debugStream = 0;
        _queuedVersions.getBack( debugStream );
        if( debugStream )
        {
            EQASSERT( debugStream->getVersion() + 1 == stream->getVersion( ));
        } 
#endif
        _queuedVersions.push( new ObjectDataIStream( *stream ));
#if 0
        EQLOG( LOG_OBJECTS ) << stream->getVersion() << ' ';
#endif
    }
#if 0
    EQLOG( LOG_OBJECTS ) << std::endl << co::base::enableFlush;
#endif
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool VersionedSlaveCM::_cmdInstance( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    EQASSERT( command.getNode().isValid( ));

    if( !_currentIStream )
        _currentIStream = _iStreamCache.alloc();

    _currentIStream->addDataPacket( command );
    if( _currentIStream->isReady( ))
    {
        const uint128_t& version = _currentIStream->getVersion();
#if 0
        EQLOG( LOG_OBJECTS ) << "v" << version << ", id " << _object->getID()
                             << "." << _object->getInstanceID() << " ready"
                             << std::endl;
#endif
#ifndef NDEBUG
        ObjectDataIStream* debugStream = 0;
        _queuedVersions.getBack( debugStream );
        if ( debugStream )
        {
            EQASSERT( debugStream->getVersion() + 1 == version );
        }
#endif
        _queuedVersions.push( _currentIStream );
        _object->notifyNewHeadVersion( version );
        _currentIStream = 0;
    }
    return true;
}

bool VersionedSlaveCM::_cmdDelta( Command& command )
{
    EQ_TS_THREAD( _cmdThread );

    if( !_currentIStream )
        _currentIStream = _iStreamCache.alloc();

    _currentIStream->addDataPacket( command );
    if( _currentIStream->isReady( ))
    {
        const uint128_t& version = _currentIStream->getVersion();
#if 0
        EQLOG( LOG_OBJECTS ) << "v" << version << ", id " << _object->getID()
                             << "." << _object->getInstanceID() << " ready"
                             << std::endl;
#endif
#ifndef NDEBUG
        ObjectDataIStream* debugStream = 0;
        _queuedVersions.getBack( debugStream );
        if ( debugStream )
        {
            EQASSERT( debugStream->getVersion() + 1 == version );
        }
#endif
        _queuedVersions.push( _currentIStream );
        _object->notifyNewHeadVersion( version );
        _currentIStream = 0;
    }
    return true;
}

bool VersionedSlaveCM::_cmdCommit( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
#if 0
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;
#endif
    LocalNodePtr localNode = _object->getLocalNode();
    if( !_master || !_master->isConnected( ))
    {
        EQASSERTINFO( false, "Master node not connected " << *_object );
        localNode->serveRequest( packet->requestID, VERSION_NONE );
        return true;
    }

    _ostream.setVersion( co::base::UUID( true )); // unique commit version
    _ostream.enable( _master, false );
    _object->pack( _ostream );
    _ostream.disable();

    localNode->serveRequest( packet->requestID,
                             _ostream.hasSentData() ? _ostream.getVersion() :
                             VERSION_NONE );
    return true;
}

}
