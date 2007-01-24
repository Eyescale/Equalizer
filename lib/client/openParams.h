
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OPEN_PARAMS_H
#define EQ_OPEN_PARAMS_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
    class EQ_EXPORT OpenParams
    {
    public:
        OpenParams();
        virtual ~OpenParams(){}

        OpenParams& operator = ( const OpenParams& rhs );

        std::string address;
        std::string appName;
    private:
    };
}

#endif // EQ_OPEN_PARAMS_H

