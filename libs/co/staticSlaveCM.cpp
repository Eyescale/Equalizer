
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

#include "staticSlaveCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "object.h"
#include "objectDataIStream.h"

#include <co/base/scopedMutex.h>

namespace co
{
typedef CommandFunc< StaticSlaveCM > CmdFunc;

StaticSlaveCM::StaticSlaveCM( Object* object )
        : ObjectCM( object )
        , _currentIStream( new ObjectDataIStream )
{
    EQASSERT( _object );
    EQASSERT( object->getLocalNode( ));

    object->registerCommand( CMD_OBJECT_INSTANCE,
                             CmdFunc( this, &StaticSlaveCM::_cmdInstance ), 0 );
}

StaticSlaveCM::~StaticSlaveCM()
{
    delete _currentIStream;
    _currentIStream = 0;
}

void StaticSlaveCM::applyMapData( const uint128_t& version )
{
    EQASSERT( _currentIStream );
    EQASSERT( version == VERSION_FIRST );
    _currentIStream->waitReady();

    EQASSERT( _object );
    EQASSERT( _currentIStream->getVersion() == VERSION_FIRST );
    EQASSERT( _currentIStream->hasInstanceData( ));

    if( _currentIStream->hasData( )) // not VERSION_NONE
        _object->applyInstanceData( *_currentIStream );

    EQASSERTINFO( !_currentIStream->hasData(),
                  "Object " << typeid( *_object ).name() <<
                  " did not unpack all data" );

    delete _currentIStream;
    _currentIStream = 0;

    EQLOG( LOG_OBJECTS ) << "Mapped initial data for " << _object->getID()
                         << "." << _object->getInstanceID() << " ready" 
                         << std::endl;
}

void StaticSlaveCM::addInstanceDatas( const ObjectDataIStreamDeque& cache,
                                      const uint128_t& /* start */ )
{
    EQ_TS_THREAD( _rcvThread );
    EQASSERT( _currentIStream );
    EQASSERT( _currentIStream->getDataSize() == 0 );
    EQASSERT( cache.size() == 1 );
    if( cache.empty( ))
        return;

    ObjectDataIStream* stream = cache.front();
    EQASSERT( stream );
    EQASSERT( stream->isReady( ));
    EQASSERT( stream->getVersion() == VERSION_FIRST );

    if( !stream->isReady() || stream->getVersion() != VERSION_FIRST )
        return;

    EQLOG( LOG_OBJECTS ) << "Adding cached instance data" << std::endl;
    delete _currentIStream;
    _currentIStream = new ObjectDataIStream( *stream );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool StaticSlaveCM::_cmdInstance( Command& command )
{
    EQ_TS_THREAD( _rcvThread );
    EQASSERT( _currentIStream );
    _currentIStream->addDataPacket( command );

    if( _currentIStream->isReady( ))
        EQLOG( LOG_OBJECTS ) << "id " << _object->getID() << "." 
                             << _object->getInstanceID() << " ready" 
                             << std::endl;

    return true;
}

}
