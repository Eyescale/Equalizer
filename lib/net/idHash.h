
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_IDHASH_H
#define EQNET_IDHASH_H

#include <eq/base/hash.h>

namespace eq
{
namespace net
{
    template<class T> class IDHash : public stde::hash_map<uint32_t, T>
    {};
}
}

#endif // EQNET_IDHASH_H
