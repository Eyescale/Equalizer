
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
        template<class T> class IDHash : public eqBase::Hash<uint, T>
        {
        public:
            bool containsKey( const uint key ) const;

            T getValue( const uint key )
                {
                    if( !containsKey( key )) return (T)0;
                    return (*this)[key];
                }
        };
    }
}

#endif // EQNET_IDHASH_H
