
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configDeserializer.h"

#include "canvas.h"
#include "config.h"
#include "global.h"
#include "layout.h"
#include "nodeFactory.h"
#include "view.h"

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
