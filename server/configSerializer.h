
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONFIGSERIALIZER_H
#define EQSERVER_CONFIGSERIALIZER_H

#include "configVisitor.h"        // base class
#include "types.h"

#include <eq/net/object.h> // base class

namespace eq
{
namespace server
{
    /** 
     * Serializes all data for a Config, which is a net::Session and can't be a
     * net::Object at the same time.
     */
    class ConfigSerializer : public net::Object
    {
    public:
        ConfigSerializer( Config* const config ) : _config( config ) {}
        virtual ~ConfigSerializer() {}

        void deregister();

    protected:
        // Use instance since then getInstanceData() is called from app thread,
        // which allows us to map all children during serialization
        virtual Object::ChangeType getChangeType() const 
            { return Object::INSTANCE; }
            
        virtual void getInstanceData( net::DataOStream& os );
        virtual void applyInstanceData( net::DataIStream& is ) {EQDONTCALL;}

    private:
        Config* const _config;
    };
}
}
#endif // EQSERVER_CONFIGSERIALIZER_H
