
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "deltaMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "objectDeltaDataOStream.h"
#include "packets.h"
#include "session.h"

namespace eq
{
namespace net
{

typedef ObjectDeltaDataOStream DeltaData;
typedef CommandFunc<DeltaMasterCM> CmdFunc;
        
DeltaMasterCM::DeltaMasterCM( Object* object )
        : FullMasterCM( object )
{
    registerCommand( CMD_OBJECT_COMMIT, 
                     CmdFunc( this, &DeltaMasterCM::_cmdCommit ), 0 );
    // sync commands are send to all instances, even the master gets it
    registerCommand( CMD_OBJECT_DELTA, 
                     CmdFunc( this, &DeltaMasterCM::_cmdDiscard ), 0 );
}

DeltaMasterCM::~DeltaMasterCM()
{
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult DeltaMasterCM::_cmdCommit( Command& command )
{
    CHECK_THREAD( _thread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;

    EQASSERT( _version != VERSION_NONE );

    ++_commitCount;

    DeltaData deltaData( _object );

    deltaData.setVersion( _version + 1 );
    deltaData.enable( _slaves );
    _object->pack( deltaData );
    deltaData.disable();

    if( !deltaData.hasSentData( ))
    {
        _obsolete();
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    // save instance data
    InstanceData* instanceData = _newInstanceData();
    instanceData->commitCount = _commitCount;
    instanceData->os.setVersion( _version + 1 );

    instanceData->os.enable();
    _object->getInstanceData( instanceData->os );
    instanceData->os.disable();

    ++_version;
    EQASSERT( _version );
    
    _instanceDatas.push_back( instanceData );
    
    _obsolete();
    _checkConsistency();

    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << std::endl;
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
}
}
