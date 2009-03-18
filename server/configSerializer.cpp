
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configSerializer.h"

#include "canvas.h"
#include "config.h"
#include "compound.h"
#include "layout.h"
#include "segment.h"
#include "view.h"

#include "../lib/client/configDeserializer.h" // header not installed

namespace eq
{
namespace server
{
namespace
{
class SerializerVisitor : public ConfigVisitor
{
public:
    SerializerVisitor( net::DataOStream& os ) : _os( os ) {}
    virtual ~SerializerVisitor(){}

    virtual VisitorResult visit( Segment* segment )
        { 
            _registerObject( segment->getConfig(), segment );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPost( Canvas* canvas )
        { 
            _registerObject( canvas->getConfig(), canvas );
            _os << eq::ConfigDeserializer::TYPE_CANVAS << canvas->getID();
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visit( View* view )
        { 
            _registerObject( view->getConfig(), view );
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visitPost( Layout* layout )
        { 
            _registerObject( layout->getConfig(), layout );
            _os << eq::ConfigDeserializer::TYPE_LAYOUT << layout->getID();
            return TRAVERSE_CONTINUE; 
        }

private:
    net::DataOStream& _os;

    void _registerObject( net::Session* session, net::Object* object )
        {
            EQASSERT( session );
            EQASSERT( object->getID() == EQ_ID_INVALID );
            
            session->registerObject( object );
        }
};
}

void ConfigSerializer::getInstanceData( net::DataOStream& os )
{
    os << _config->getLatency() 
       << _config->getFAttribute( Config::FATTR_EYE_BASE );

    SerializerVisitor serializer( os );
    _config->accept( serializer );
    os << eq::ConfigDeserializer::TYPE_LAST; // end token

#ifdef EQ_TRANSMISSION_API
#  error TODO transmit node identifiers of used nodes
#endif
}

}
}
