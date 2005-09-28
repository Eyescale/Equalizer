
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_PARAMS_H
#define EQ_CONFIG_PARAMS_H

#include <string>

namespace eq
{
#   define COMPOUND_MODE_2D    0x01
#   define COMPOUND_MODE_DPLEX 0x02
#   define COMPOUND_MODE_EYE   0x04
#   define COMPOUND_MODE_FSAA  0x08
#   define COMPOUND_MODE_CULL  0x10
#   define COMPOUND_MODE_DB    0x20
#   define COMPOUND_MODE_3D    0x40

    class ConfigParams
    {
    public:
        ConfigParams();
        virtual ~ConfigParams(){}

        ConfigParams& operator = ( const ConfigParams& rhs );

        std::string appName;
        std::string renderClient;
        uint        compoundModes;
    private:
    };
}

#endif // EQ_CONFIG_PARAMS_H

