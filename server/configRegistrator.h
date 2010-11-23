
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
                // double commit on update/delete
                return _register( observer, _config->getLatency() + 1 );
            }

        virtual VisitorResult visit( Segment* segment )
            { 
                // double commit on update/delete
                return _register( segment, _config->getLatency() + 1 );
            }

        virtual VisitorResult visitPost( Canvas* canvas )
            { 
                // double commit on update/delete
                return _register( canvas, _config->getLatency() + 1 );
            }

        virtual VisitorResult visit( View* view )
            {
                // double commit on update/delete
                return _register( view, _config->getLatency() + 1 );
            }

        virtual VisitorResult visitPost( Layout* layout )
            { 
                // double commit on update/delete
                return _register( layout, _config->getLatency() + 1 );
            }

        virtual VisitorResult visit( Channel* channel )
            {
                return _register( channel, 0 );
            }
        virtual VisitorResult visitPost( Window* window )
            {
                return _register( window, 0 );
            }
        virtual VisitorResult visitPost( Pipe* pipe )
            {
                return _register( pipe, 0 );
            }
        virtual VisitorResult visitPost( Node* node )
            {
                return _register( node, 0 );
            }

        virtual VisitorResult visit( Compound* compound )
            {
                compound->register_();
                return TRAVERSE_CONTINUE; 
            }

    private:
        Config* const _config;

        VisitorResult _register( net::Object* object, const uint32_t nBuffers )
            {
                EQASSERT( object->getID() == base::EQ_UUID_INVALID );
                _config->registerObject( object );
                if( nBuffers > 0 )
                    object->setAutoObsolete( nBuffers );
                return TRAVERSE_CONTINUE; 
            }
    };
}
}
}
#endif // EQSERVER_CONFIGREGISTRATOR_H
