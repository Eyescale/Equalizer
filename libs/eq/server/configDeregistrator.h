
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CONFIGDEREGISTRATOR_H
#define EQSERVER_CONFIGDEREGISTRATOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
    /** Unmaps all mapped config children. */
    class ConfigDeregistrator : public ConfigVisitor
    {
    public:
        virtual ~ConfigDeregistrator(){}

        virtual VisitorResult visitPost( Canvas* canvas )
            {
                _deregister( canvas );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visit( Segment* segment )
            { 
                _deregister( segment );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visitPost( Layout* layout )
            { 
                _deregister( layout );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visit( View* view )
            { 
                _deregister( view );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visit( Observer* observer )
            { 
                _deregister( observer );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visitPost( Config* config )
            { 
                _deregister( config );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPost( Node* node )
            { 
                _deregister( node );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPost( Pipe* pipe )
            { 
                _deregister( pipe );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPost( Window* window )
            { 
                _deregister( window );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visit( Channel* channel )
            { 
                _deregister( channel );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visit( Compound* compound )
            {
                compound->deregister();
                return TRAVERSE_CONTINUE; 
            }

    private:
        void _deregister( co::Object* object )
            {
                EQASSERT( object->isAttached( ));
                if( !object->isAttached( ))
                    return;
                EQASSERT( object->isMaster( ));

                object->sync();
                co::LocalNodePtr node = object->getLocalNode();
                node->releaseObject( object );
            }
    };
}

}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
