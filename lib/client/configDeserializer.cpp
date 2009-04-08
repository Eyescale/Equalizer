
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    EQASSERT( _config->_canvases.empty( ));
    for( CanvasVector::const_iterator i = _config->_canvases.begin();
         i != _config->_canvases.end(); ++i )
    {
        Canvas* canvas = *i;
        canvas->deregister();
        nodeFactory->releaseCanvas( canvas );
    }
    _config->_canvases.clear();

    EQASSERT( _config->_layouts.empty( ));
    for( LayoutVector::const_iterator i = _config->_layouts.begin();
         i != _config->_layouts.end(); ++i )
    {
        Layout* layout = *i;
        layout->deregister();
        nodeFactory->releaseLayout( layout );
    }
    _config->_layouts.clear();

    // map all config children as master
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
                Observer* observer = nodeFactory->createObserver();
                EQASSERT( observer );
                _config->_addObserver( observer );

                EQCHECK( _config->mapObject( observer, id )); //OPT: async map
                observer->becomeMaster();
                break;
            }
                
            case TYPE_CANVAS:
            {
                Canvas* canvas = nodeFactory->createCanvas();
                EQASSERT( canvas );
                _config->_addCanvas( canvas );

                EQCHECK( _config->mapObject( canvas, id )); //OPT: async mapping
                canvas->becomeMaster();
                break;
            }

            case TYPE_LAYOUT:
            {
                Layout* layout = nodeFactory->createLayout();
                EQASSERT( layout );
                _config->_addLayout( layout );

                EQCHECK( _config->mapObject( layout, id )); //OPT: async mapping
                // RO, don't: layout->becomeMaster();
                break;
            }
                
            default:
                EQUNIMPLEMENTED;
        }
    }
}

}
