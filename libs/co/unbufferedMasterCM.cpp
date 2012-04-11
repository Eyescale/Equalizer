
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "unbufferedMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "nodePackets.h"
#include "object.h"
#include "objectDataIStream.h"
#include "objectDeltaDataOStream.h"
#include "objectInstanceDataOStream.h"
#include "objectPackets.h"

namespace co
{
typedef CommandFunc<UnbufferedMasterCM> CmdFunc;

UnbufferedMasterCM::UnbufferedMasterCM( Object* object )
        : MasterCM( object )
{
    _version = VERSION_FIRST;
    LBASSERT( object );
    LBASSERT( object->getLocalNode( ));
}

UnbufferedMasterCM::~UnbufferedMasterCM()
{}

uint128_t UnbufferedMasterCM::commit( const uint32_t incarnation )
{
    Mutex mutex( _slaves );
#if 0
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command
                         << std::endl;
#endif
    if( !_object->isDirty() || _slaves->empty( ))
        return _version;

    ObjectDeltaDataOStream os( this );
    os.enableCommit( _version + 1, *_slaves );
    _object->pack( os );
    os.disable();

    if( os.hasSentData( ))
    {
        ++_version;
        LBASSERT( _version != VERSION_NONE );
#if 0
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << std::endl;
#endif
    }

    return _version;
}

}
