
/* Copyright (c) 2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#ifndef EQSERVER_SETFAILEDVISITOR_H
#define EQSERVER_SETFAILEDVISITOR_H
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
class SetFailedVisitor : public ConfigVisitor
{
public:
    virtual ~SetFailedVisitor(){}

    virtual VisitorResult visitPost( Node* node )
        { 
            node->setFailed( );
            return TRAVERSE_TERMINATE; 
        }

    virtual VisitorResult visit( Channel* channel )
        { 
            channel->setFailed( );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Pipe* pipe )
        { 
            pipe->setFailed( );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Window* window )
        { 
            window->setFailed( );
            return TRAVERSE_CONTINUE; 
        }
};

}

}
#endif // EQSERVER_SETFAILEDVISITOR_H
