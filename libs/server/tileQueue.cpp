
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
 *               2011, Daniel Nachbaur <danielnachbaur@googlemail.com>
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

#include "tileQueue.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>


namespace eq
{
namespace server
{

TileQueue::TileQueue()
        : _compound( 0 )
        , _size( 0, 0 )
        , _queueMaster()
{
}

TileQueue::TileQueue( const TileQueue& from )
        : co::Object()
        , _compound( 0 )
        , _name( from._name )
        , _size( from._size )
{
}

TileQueue::~TileQueue()
{
    _compound = 0;
}

void TileQueue::addTile( const TileTaskPacket& tile )
{
    _queueMaster.push( tile );
}

void TileQueue::getInstanceData( co::DataOStream& )
{
}

void TileQueue::applyInstanceData( co::DataIStream& )
{
}

void TileQueue::flush()
{
    unsetData();
}

void TileQueue::unsetData()
{
    _queueMaster.clear();
}

std::ostream& operator << ( std::ostream& os, const TileQueue* tileQueue )
{
    if( !tileQueue )
        return os;
    
    os << co::base::disableFlush << "tileQueue" << std::endl;
    os << "{" << std::endl << co::base::indent;
      
    const std::string& name = tileQueue->getName();
    os << "name      \"" << name << "\"" << std::endl;

    const eq::Vector2i& size = tileQueue->getTileSize();
    os << "tile size \"" << size << "\"" << std::endl;

    os << co::base::exdent << "}" << std::endl << co::base::enableFlush;
    return os;
}

}
}
