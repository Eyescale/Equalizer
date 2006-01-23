
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OPEN_PARAMS_H
#define EQ_OPEN_PARAMS_H

#include <string>

namespace eq
{
    class OpenParams
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

