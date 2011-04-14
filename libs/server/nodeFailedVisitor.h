
/* Copyright (c) 2011, Cedric Stalder<cedric.stalder@gmail.com>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSERVER_NODEFAILEDVISITOR_H
#define EQSERVER_NODEFAILEDVISITOR_H

#include "visitorResult.h" // 'base' class
#include "pipe.h" // member
#include "node.h" // member
#include "window.h" // member
#include "channel.h" // member

namespace eq
{
namespace server
{
    
/** Set Failed State all node branch. */
class NodeFailedVisitor : public ConfigVisitor
{
public:
    virtual ~NodeFailedVisitor(){}

    virtual VisitorResult visitPre( Node* node )
        {
            _updateState( node );

            co::LocalNodePtr localNode = node->getLocalNode();
            co::NodePtr netNode = node->getNode();
            localNode->disconnect( netNode );
            node->setNode( 0 );

            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Pipe* pipe )
        { 
            _updateState( pipe );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Window* window )
        {
            _updateState( window );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visit( Channel* channel )
        {
            _updateState( channel );
            return TRAVERSE_CONTINUE; 
        }

private:
    template< class T > void _updateState( T* entity )
        {
            entity->setState( entity->isActive() ? STATE_FAILED:STATE_STOPPED );
        }
};

}

}
#endif // EQSERVER_NODEFAILEDVISITOR_H
