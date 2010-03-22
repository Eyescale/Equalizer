
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "configDeserializer.h"

#include "canvas.h"
#include "config.h"
#include "global.h"
#include "layout.h"
#include "nodeFactory.h"
#include "observer.h"
#include "view.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
void ConfigDeserializer::applyInstanceData( net::DataIStream& is )
{
    is >> _config->_latency >> _config->_eyeBase;

    NodeFactory* nodeFactory = Global::getNodeFactory();
    
    // Clean up - should never be necessary
    const ObserverVector& observers = _config->getObservers();
    EQASSERT( observers.empty( ));
    while( !observers.empty( ))
    {
        Observer* observer = observers.back();
        //observer->_deregister();
        nodeFactory->releaseObserver( observer );
    }

    const CanvasVector& canvases = _config->getCanvases();
    EQASSERT( canvases.empty( ));
    for( CanvasVector::const_iterator i = canvases.begin();
         i != canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        nodeFactory->releaseCanvas( canvas );
    }

    const LayoutVector& layouts = _config->getLayouts();
    EQASSERT( layouts.empty( ));
    while( !layouts.empty( ))
    {
        Layout* layout = layouts.back();
        //layout->_deregister();
        nodeFactory->releaseLayout( layout );
    }

    // map all config children
    net::ObjectVector objects; // save objects to convert to master
    Type type;
    for( is >> type; type != TYPE_LAST; is >> type )
    {
        uint32_t id;
        is >> id;
        EQASSERT( id != EQ_ID_INVALID );

        switch( type )
        {
            case TYPE_OBSERVER:
            {
                Observer* observer = nodeFactory->createObserver( _config );
                EQASSERT( observer );
                EQCHECK( _config->mapObject( observer, id )); //OPT: async map
                break;
            }
                
            case TYPE_CANVAS:
            {
                Canvas* canvas = nodeFactory->createCanvas();
                EQASSERT( canvas );
                _config->_addCanvas( canvas );

                EQCHECK( _config->mapObject( canvas, id )); //OPT: async mapping
                objects.push_back( canvas );
                break;
            }

            case TYPE_LAYOUT:
            {
                Layout* layout = nodeFactory->createLayout( _config );
                EQASSERT( layout );
                EQCHECK( _config->mapObject( layout, id )); //OPT: async mapping
                break;
            }
                
            default:
                EQUNIMPLEMENTED;
        }
    }

    // Convert objects to master after all the mapping has happened, since the
    // objects refer to each other using identifiers during deserialization.
    for( net::ObjectVector::const_iterator i = objects.begin();
         i != objects.end(); ++i )
    {
        (*i)->becomeMaster();
    }
}

}
