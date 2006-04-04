
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_LOADER_CLOSURE_H
#define EQS_LOADER_CLOSURE_H

#include "loader.h"
#include "loaderState.h"

#include <boost/spirit/attribute.hpp>

namespace eqLoader
{
    struct grammarClosure : boost::spirit::closure<grammarClosure, State>
    {
        member1 state;
    };
}

#endif // EQS_LOADER_CLOSURE_H
