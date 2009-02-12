
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configSerializer.h"

#include "config.h"
#include "compound.h"
#include "view.h"

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

    virtual VisitorResult visit( Compound* compound )
        { 
            // TODO: belongs to observer
            Config* config = compound->getConfig();
            compound->getFrustum().setEyeBase(
                config->getFAttribute( Config::FATTR_EYE_BASE ));
            return TRAVERSE_CONTINUE; 
        }

    virtual VisitorResult visit( View* view )
        { 
            Config* config = view->getConfig();
            EQASSERT( config );
            EQASSERT( view->getID() == EQ_ID_INVALID );
            
            config->registerObject( view );
            _os << view->getID();
            return TRAVERSE_CONTINUE; 
        }

private:
    net::DataOStream& _os;
};
}

void ConfigSerializer::getInstanceData( net::DataOStream& os )
{
    os << _config->getLatency();

    SerializerVisitor serializer( os );
    _config->accept( &serializer );
    os << EQ_ID_INVALID; // end token

#ifdef EQ_TRANSMISSION_API
#  error TODO transmit node identifiers of used nodes
#endif
}

namespace
{
class UnmapVisitor : public ConfigVisitor
{
public:
    virtual ~UnmapVisitor(){}

    virtual VisitorResult visit( View* view )
        { 
            if( view->getID() != EQ_ID_INVALID )
            {
                Config* config = view->getConfig();
                EQASSERT( config );
                config->unmapObject( view );
            }

            return TRAVERSE_CONTINUE; 
        }
};
}

void ConfigSerializer::deregister()
{
    UnmapVisitor unmapper;
    _config->accept( &unmapper );
    _config->deregisterObject( this );
}

}
}
