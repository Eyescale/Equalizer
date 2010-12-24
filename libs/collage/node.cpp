
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "node.h"

#include "connectionDescription.h"
#include "nodePackets.h"

#include <co/base/os.h>

namespace co
{

Node::Node()
        : _id( true )
        , _state( STATE_CLOSED )
{
    EQVERB << "New Node @" << (void*)this << " " << _id << std::endl;
}

Node::~Node()
{
    EQVERB << "Delete Node @" << (void*)this << " " << _id << std::endl;
    EQASSERT( _outgoing == 0 );
    _connectionDescriptions->clear();
}

bool Node::operator == ( const Node* node ) const
{ 
    EQASSERTINFO( _id != node->_id || this == node,
                  "Two node instances with the same ID found "
                  << (void*)this << " and " << (void*)node );

    return ( this == node );
}

ConnectionDescriptions Node::getConnectionDescriptions() const
{
    co::base::ScopedMutex< co::base::SpinLock > mutex( _connectionDescriptions );
    return _connectionDescriptions.data;
}

ConnectionPtr Node::getMulticast()
{
    EQASSERT( isConnected( ));

    if( !isConnected( ))
        return 0;
    
    ConnectionPtr connection = _outMulticast.data;
    if( connection.isValid() && !connection->isClosed( ))
        return connection;

    co::base::ScopedMutex<> mutex( _outMulticast );
    if( _multicasts.empty( ))
        return 0;

    MCData data = _multicasts.back();
    _multicasts.pop_back();
    NodePtr node = data.node;

    // prime multicast connections on peers
    EQINFO << "Announcing id " << node->getNodeID() << " to multicast group "
           << data.connection->getDescription() << std::endl;

    NodeIDPacket packet;
    packet.id = node->getNodeID();
    packet.nodeType = getType();

    data.connection->send( packet, node->serialize( ));
    _outMulticast.data = data.connection;
    return data.connection;
}

void Node::addConnectionDescription( ConnectionDescriptionPtr cd )
{
    if( cd->type >= CONNECTIONTYPE_MULTICAST && cd->port == 0 )
        cd->port = EQ_DEFAULT_PORT;

    co::base::ScopedMutex< co::base::SpinLock > 
                       mutex( _connectionDescriptions );
    _connectionDescriptions->push_back( cd ); 
}

bool Node::removeConnectionDescription( ConnectionDescriptionPtr cd )
{
    co::base::ScopedMutex< co::base::SpinLock > 
                       mutex( _connectionDescriptions );

    // Don't use std::find, RefPtr::operator== compares pointers, not values.
    for( ConnectionDescriptions::iterator i = _connectionDescriptions->begin();
         i != _connectionDescriptions->end(); ++i )
    {
        if( *cd != **i )
            continue;

        _connectionDescriptions->erase( i );
        return true;
    }
    return false;
}

std::string Node::serialize() const
{
    std::ostringstream data;
    {
        co::base::ScopedMutex< co::base::SpinLock >
                                            mutex( _connectionDescriptions );
        data << _id << CO_SEPARATOR
             << co::serialize( _connectionDescriptions.data );
    }
    return data.str();
}
 
bool Node::deserialize( std::string& data )
{
    EQASSERT( _state == STATE_CLOSED );

    // node id
    size_t nextPos = data.find( CO_SEPARATOR );
    if( nextPos == std::string::npos || nextPos == 0 )
    {
        EQERROR << "Could not parse node data" << std::endl;
        return false;
    }

    _id = data.substr( 0, nextPos );
    data = data.substr( nextPos + 1 );

    co::base::ScopedMutex< co::base::SpinLock > 
                    mutex( _connectionDescriptions );
    _connectionDescriptions->clear();
    return co::deserialize( data, _connectionDescriptions.data );
}

NodePtr Node::createNode( const uint32_t type )
{
    EQASSERTINFO( type == NODETYPE_CO_NODE, type );
    return new Node();
}

std::ostream& operator << ( std::ostream& os, const Node& node )
{
    os << "node " << node.getNodeID() << " " << node._state;
    const ConnectionDescriptions& descs = node.getConnectionDescriptions();
    for( ConnectionDescriptions::const_iterator i = descs.begin();
         i != descs.end(); ++i )
    {
        os << ", " << (*i)->toString();
    }
    return os;
}

std::ostream& operator << ( std::ostream& os, const Node::State state)
{
    os << ( state == Node::STATE_CLOSED ? "closed" :
            state == Node::STATE_CONNECTED ? "connected" :
            state == Node::STATE_LISTENING ? "listening" : "ERROR" );
    return os;
}

}
