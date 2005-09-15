
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/base/base.h>

namespace eq
{
    class Config
    {
    public:
        /** 
         * Constructs a new config.
         * 
         * @param id the server-supplied identifier of the config.
         */
        Config( const uint id );

    private:
        const uint _id;
    };
}

#endif // EQ_CONFIG_H

