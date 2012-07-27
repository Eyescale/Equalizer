
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "node.h"

#include "elementVisitor.h"
#include "leafVisitor.h"
#include "log.h"
#include "paths.h"

#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{
namespace fabric
{
namespace
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_NODE_") + #attr )

std::string _iAttributeStrings[] = {
    MAKE_ATTR_STRING( IATTR_THREAD_MODEL ),
    MAKE_ATTR_STRING( IATTR_LAUNCH_TIMEOUT ),
    MAKE_ATTR_STRING( IATTR_HINT_AFFINITY )
};

}

template< class C, class N, class P, class V >
Node< C, N, P, V >::Node( C* parent )  
        : _config( parent )
        , _isAppNode( false )
{
    parent->_addNode( static_cast< N* >( this ) );
    LBLOG( LOG_INIT ) << "New " << lunchbox::className( this ) << std::endl;
}

template< class C, class N, class P, class V >
Node< C, N, P, V >::~Node()
{
    LBLOG( LOG_INIT ) << "Delete " << lunchbox::className( this ) << std::endl;
    while( !_pipes.empty() )
    {
        P* pipe = _pipes.back();
        LBASSERT( pipe->getNode() == static_cast< N* >( this ) );
        _removePipe( pipe );
        delete pipe;
    }

    _config->_removeNode( static_cast< N* >( this ) );
}

template< class C, class N, class P, class V >
Node< C, N, P, V >::BackupData::BackupData()
{
    memset( iAttributes, 0xff, IATTR_ALL * sizeof( int32_t ));
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::backup()
{
    Object::backup();
    _backup = _data;
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::restore()
{
    _data = _backup;
    Object::restore();
}

template< class C, class N, class P, class V >
uint128_t Node< C, N, P, V >::commit( const uint32_t incarnation )
{
    if( Serializable::isDirty( DIRTY_PIPES ))
        commitChildren( _pipes, incarnation );
    return Object::commit( incarnation );
}

template< class C, class N, class P, class V > void
Node< C, N, P, V >::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    Object::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        os.write( _data.iAttributes, IATTR_ALL * sizeof( int32_t ));
    if( dirtyBits & DIRTY_PIPES && isMaster( ))
    {
        os << _mapNodeObjects();
        os.serializeChildren( _pipes );
    }
    if( dirtyBits & DIRTY_MEMBER )
        os << _isAppNode;
}

template< class C, class N, class P, class V > void
Node< C, N, P, V >::deserialize( co::DataIStream& is, const uint64_t dirtyBits)
{
    Object::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_ATTRIBUTES )
        is >> co::Array< int32_t >( _data.iAttributes, IATTR_ALL );
    if( dirtyBits & DIRTY_PIPES )
    {
        if( isMaster( ))
            syncChildren( _pipes );
        else
        {
            bool useChildren;
            is >> useChildren;
            if( useChildren && _mapNodeObjects( ))
            {
                Pipes result;
                is.deserializeChildren( this, _pipes, result );
                _pipes.swap( result );
                LBASSERT( _pipes.size() == result.size( ));
            }
            else // consume unused ObjectVersions
            {
                co::ObjectVersions childIDs;
                is >> childIDs;
            }
        }
    }
    if( dirtyBits & DIRTY_MEMBER )
        is >> _isAppNode;
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::setDirty( const uint64_t dirtyBits )
{
    Object::setDirty( dirtyBits );
    _config->setDirty( C::DIRTY_NODES );
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::notifyDetach()
{
    Object::notifyDetach();
    while( !_pipes.empty( ))
    {
        P* pipe = _pipes.back();
        if( !pipe->isAttached( ))
        {
            LBASSERT( isMaster( ));
            return;
        }

        LBASSERT( !isMaster( ));
        getLocalNode()->unmapObject( pipe );
        _removePipe( pipe );
        _config->getServer()->getNodeFactory()->releasePipe( pipe );
    }
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::create( P** pipe )
{
    *pipe = _config->getServer()->getNodeFactory()->createPipe( 
        static_cast< N* >( this ));
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::release( P* pipe )
{
    _config->getServer()->getNodeFactory()->releasePipe( pipe );
}

namespace
{
template< class N, class V >
VisitorResult _accept( N* node, V& visitor )
{
    VisitorResult result = visitor.visitPre( node );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const typename N::Pipes& pipes = node->getPipes();
    for( typename N::Pipes::const_iterator i = pipes.begin(); 
         i != pipes.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( node ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

template< class C, class N, class P, class V >
VisitorResult Node< C, N, P, V >::accept( V& visitor )
{
    return _accept( static_cast< N* >( this ), visitor );
}

template< class C, class N, class P, class V >
VisitorResult Node< C, N, P, V >::accept( V& visitor ) const
{
    return _accept( static_cast< const N* >( this ), visitor );
}

template< class C, class N, class P, class V >
NodePath Node< C, N, P, V >::getPath() const
{
    const C* config = static_cast< const N* >( this )->getConfig( );
    LBASSERT( config );
    
    const typename std::vector< N* >& nodes = config->getNodes();
    typename std::vector< N* >::const_iterator i =
        std::find( nodes.begin(), nodes.end(), this );
    LBASSERT( i != nodes.end( ));

    NodePath path;
    path.nodeIndex = std::distance( nodes.begin(), i );
    return path;
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::setApplicationNode( const bool isAppNode )
{
    if( _isAppNode == isAppNode )
        return;
    _isAppNode = isAppNode;
    setDirty( DIRTY_MEMBER );
}

template< class C, class N, class P, class V > void
Node< C, N, P, V >::setIAttribute( const IAttribute attr, const int32_t value )
{
    if( _data.iAttributes[attr] == value )
        return;
    _data.iAttributes[attr] = value;
    setDirty( DIRTY_ATTRIBUTES );
}

template< class C, class N, class P, class V >
int32_t Node< C, N, P, V >::getIAttribute( const IAttribute attr ) const
{
    return _data.iAttributes[attr];
}

template< class C, class N, class P, class V > const std::string&
Node< C, N, P, V >::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

template< class C, class N, class P, class V >
void Node< C, N, P, V >::_addPipe( P* pipe )
{
    LBASSERT( pipe->getNode() == this );
    _pipes.push_back( pipe );
}

template< class C, class N, class P, class V >
bool Node< C, N, P, V >::_removePipe( P* pipe )
{
    typename Pipes::iterator i = stde::find( _pipes, pipe );
    if( i == _pipes.end( ))
        return false;

    _pipes.erase( i );
    return true;
}

template< class C, class N, class P, class V >
P* Node< C, N, P, V >::findPipe( const UUID& id )
{
    for( typename Pipes::const_iterator i = _pipes.begin();
         i != _pipes.end(); ++i )
    {
        P* pipe = *i;
        if( pipe->getID() == id )
            return pipe;
    }
    return 0;
}

template< class C, class N, class P, class V >
std::ostream& operator << ( std::ostream& os, const Node< C, N, P, V >& node )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader;
    if( node.isApplicationNode( ))
        os << "appNode" << std::endl;
    else
        os << "node" << std::endl;

    os << "{" << std::endl << lunchbox::indent;

    const std::string& name = node.getName();
    if( !name.empty( ))
        os << "name     \"" << name << "\"" << std::endl;

    node.output( os );

    const typename N::Pipes& pipes = node.getPipes();
    for( typename N::Pipes::const_iterator i = pipes.begin();
         i != pipes.end(); ++i )
    {
        os << **i;
    }
    os << lunchbox::exdent << "}" << std::endl
       << lunchbox::enableHeader << lunchbox::enableFlush;
    return os;
}

}
}
