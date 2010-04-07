
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSERVER_CONFIGREGISTRATOR_H
#define EQSERVER_CONFIGREGISTRATOR_H

#include "configVisitor.h"        // base class
#include "types.h"

namespace eq
{
namespace server
{
namespace
{
    /** Registers all objects of a config. @internal */
    class ConfigRegistrator : public ConfigVisitor
    {
    public:
        ConfigRegistrator( Config* config ) : _config( config ) {}
        virtual ~ConfigRegistrator(){}

        virtual VisitorResult visit( Observer* observer )
            { 
                _register( observer );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visit( Segment* segment )
            { 
                _register( segment );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visitPost( Canvas* canvas )
            { 
                _register( canvas );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visit( View* view )
            {
                _register( view );
                view->setAutoObsolete( _config->getLatency( ));
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visitPost( Layout* layout )
            { 
                _register( layout );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visit( Channel* channel )
            {
                _register( channel );
                return TRAVERSE_CONTINUE; 
            }

        virtual VisitorResult visitPost( Window* window )
            {
                _register( window );
                return TRAVERSE_CONTINUE; 
            }

    private:
        Config* const _config;

        void _register( net::Object* object )
            {
                EQASSERT( object->getID() == EQ_ID_INVALID );
                _config->registerObject( object );
            }
    };
}
}
}
#endif // EQSERVER_CONFIGREGISTRATOR_H
