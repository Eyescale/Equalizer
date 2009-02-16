
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configDeserializer.h"

namespace eq
{
void ConfigDeserializer::applyInstanceData( net::DataIStream& is )
{
    is >> _config->_latency;

    EQASSERT( _config->_views.empty( ));
    for( ViewVector::const_iterator i = _config->_views.begin();
         i != _config->_views.end(); ++i )
    {
        _config->deregisterObject( *i );
        delete *i;
    }
    _config->_views.clear();

    uint32_t viewID;
    is >> viewID;
    while( viewID != EQ_ID_INVALID )
    {
        View* view = new View(); 

        _config->mapObject( view, viewID ); // OPT: async mapping
        view->becomeMaster();

        _config->_views.push_back( view );
        is >> viewID;
    }
}

}
