
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_IDHASH_H
#define EQNET_IDHASH_H

#include <eq/base/hash.h>

namespace eqNet
{
    /** The namespace for the private implementation of the eqNet classes. */
    namespace priv
    {
        template<class T> class IDHash : public Sgi::hash_map<uint, T>
        {};
    }
}

#endif // EQNET_IDHASH_H
