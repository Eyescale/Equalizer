
/* Copyright (c) 2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "dataIStreamQueue.h"

#include "objectPackets.h"
#include "objectDataIStream.h"

namespace co
{
DataIStreamQueue::DataIStreamQueue()
{}

DataIStreamQueue::~DataIStreamQueue()
{
    EQASSERTINFO( _pending.empty(), "Incomplete commits pending" );
    EQASSERTINFO( _queued.isEmpty(), _queued.getSize() << " unapplied commits" )

    for( PendingStreamsCIter i = _pending.begin(); i != _pending.end(); ++i )
        delete i->second;
    _pending.clear();

    ObjectDataIStream* is = 0;
    while( _queued.tryPop( is ))
        delete is;
}

ObjectDataIStream* DataIStreamQueue::tryPop()
{
    ObjectDataIStream* stream = 0;
    _queued.tryPop( stream );
    return stream;
}

ObjectDataIStream* DataIStreamQueue::pull( const uint128_t& version )
{
    ObjectDataIStream* is = 0;
    ObjectDataIStreams unusedStreams;
    while( !is )
    {
        ObjectDataIStream* candidate = _queued.pop();
        if( candidate->getVersion() == version )
            is = candidate;
        else
            unusedStreams.push_back( candidate );
    }

    _queued.pushFront( unusedStreams );
    return is;
}

bool DataIStreamQueue::addDataPacket( const uint128_t& key, Command& command )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( _pending.size() < 100, "More than 100 pending commits");

    ObjectDataIStream* istream = 0;
    PendingStreams::iterator i = _pending.find( key );
    if( i == _pending.end( ))
        istream = _iStreamCache.alloc();
    else
        istream = i->second;

    istream->addDataPacket( command );
    if( istream->isReady( ))
    {
        if( i != _pending.end( ))
            _pending.erase( i );

        _queued.push( istream );
        EQASSERTINFO( _queued.getSize() < 100, "More than 100 queued commits" );
        //EQLOG( LOG_OBJECTS ) << "Queued commit " << key << std::endl;
        return true;
    }

    if( i == _pending.end( ))
    {
        _pending[ key ] = istream;
        //EQLOG( LOG_OBJECTS ) << "New incomplete commit " << key << std::endl;
        return false;
    }

    //EQLOG(LOG_OBJECTS) << "Add data to incomplete commit " << key <<std::endl;
    return false;
}

}
