
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_IDHASH_H
#define EQNET_IDHASH_H

#include <eq/base/base.h>

#ifdef __GNUC__              // GCC 3.1 and later
#  include <ext/hash_map>
namespace Sgi = ::__gnu_cxx; 
#else                        //  other compilers
#  include <hash_map>
namespace Sgi = std;
#endif

namespace eqNet
{
    /** The namespace for the private implementation of the eqNet classes. */
    namespace priv
    {
        template<class T> class IDHash : public Sgi::hash_map<uint, T>
        {
        public:
            //IDHash() : Sgi::hash_map<uint, T>() {}
        };
    }
}

#endif // EQNET_IDHASH_H
