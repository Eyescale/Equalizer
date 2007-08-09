
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_HASH_H
#define EQBASE_HASH_H

#include <eq/base/base.h>
#include <eq/base/stdExt.h>

namespace eqBase
{
    /** A hash for pointer keys. */
    template<class K, class T> class PtrHash 
#ifdef WIN32_VC
        : public stde::hash_map< K, T, stde::hash_compare< const void* > >
#else
        : public stde::hash_map< K, T, stde::hash< const void* > >
#endif
    {};
}

#endif // EQBASE_HASH_H
