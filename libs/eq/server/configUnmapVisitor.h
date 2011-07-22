
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 *               2010, Cedric Stalder<cedric.stalder@gmail.com>
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

#ifndef EQSERVER_CONFIGUNMAPVISITOR_H
#define EQSERVER_CONFIGUNMAPVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
/** Unmaps all mapped config children. */
class ConfigUnmapVisitor : public ConfigVisitor
{
public:
    virtual ~ConfigUnmapVisitor(){}

    virtual VisitorResult visitPre( Canvas* canvas )
        {
            _unmap( canvas );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visit( Segment* segment )
        { 
            _unmap( segment );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPre( Layout* layout )
        { 
            _unmap( layout );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visit( View* view )
        { 
            _unmap( view );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visit( Observer* observer )
        { 
            _unmap( observer );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visit( Channel* channel )
        { 
            _unmap( channel );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visitPre( Window* window )
        { 
            _unmap( window );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visitPre( Pipe* pipe )
        { 
            _unmap( pipe );
            return TRAVERSE_CONTINUE; 
        }
    virtual VisitorResult visitPre( Node* node )
        { 
            _unmap( node );
            return TRAVERSE_CONTINUE; 
        }

private:
    void _unmap( net::Object* object )
        {
            if( !object->isAttached( ))
                return;

            net::Session* session = object->getSession();
            EQASSERT( session );

            EQASSERT( object->isMaster( ));
            session->releaseObject( object );
        }

};
}

}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
