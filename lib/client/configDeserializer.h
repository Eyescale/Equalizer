
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIGDESERIALIZER_H
#define EQ_CONFIGDESERIALIZER_H

#include <eq/net/object.h>

namespace eq
{
    class Config;

    /** Helper class to receive a config, which is a net::Session */
    class ConfigDeserializer : public net::Object
    {
    public:
        ConfigDeserializer( Config* config ) : _config( config ) {}
        virtual ~ConfigDeserializer() {}

    protected:
        virtual void getInstanceData( net::DataOStream& os ) { EQDONTCALL; }
        virtual void applyInstanceData( net::DataIStream& is );

    private:
        Config* const _config;
    };
}

#endif // EQ_CONFIGDESERIALIZER_H
