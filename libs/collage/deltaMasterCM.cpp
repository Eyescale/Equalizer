
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include "objectDataIStream.h"
#include "objectPackets.h"

namespace co
{

typedef CommandFunc<DeltaMasterCM> CmdFunc;
        
DeltaMasterCM::DeltaMasterCM( Object* object )
        : FullMasterCM( object )
#pragma warning(push)
#pragma warning(disable : 4355)
        , _deltaData( this )
#pragma warning(pop)
{
    EQASSERT( object );
    EQASSERT( object->getLocalNode( ));
    CommandQueue* q = object->getLocalNode()->getCommandThreadQueue();

    object->registerCommand( CMD_OBJECT_COMMIT, 
                             CmdFunc( this, &DeltaMasterCM::_cmdCommit ), q );
    object->registerCommand( CMD_OBJECT_DELTA, 
                             CmdFunc( this, &DeltaMasterCM::_cmdDiscard ), q );
}

DeltaMasterCM::~DeltaMasterCM()
{
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool DeltaMasterCM::_cmdCommit( Command& command )
{
    EQ_TS_THREAD( _cmdThread );
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
#if 0
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command 
                         << std::endl;
#endif

    EQASSERT( _version != VERSION_NONE );

    ++_commitCount;

    if( packet->requestID != EQ_UNDEFINED_UINT32 )
    {
        if( !_slaves.empty( ))
        {
            _deltaData.reset();
            _deltaData.setVersion( _version + 1 );
            _deltaData.enable( _slaves );
            _object->pack( _deltaData );
            _deltaData.disable();
        }

        LocalNodePtr localNode = _object->getLocalNode();
        if( _slaves.empty() || _deltaData.hasSentData( ))
        {
            // save instance data
            InstanceData* instanceData = _newInstanceData();
            instanceData->commitCount = _commitCount;
            instanceData->os.setVersion( _version + 1 );

            instanceData->os.enable();
            _object->getInstanceData( instanceData->os );
            instanceData->os.disable();

            if( _deltaData.hasSentData() || instanceData->os.hasSentData( ))
            {
                ++_version;
                EQASSERT( _version != VERSION_NONE );

                _addInstanceData( instanceData );
            }
            else
                _releaseInstanceData( instanceData );

#if 0
            EQLOG( LOG_OBJECTS ) << "Committed v" << _version << " " 
                                 << *_object << std::endl;
#endif
        }
        localNode->serveRequest( packet->requestID, _version );
    }

    _obsolete();
    return true;
}

}
