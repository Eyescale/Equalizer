
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/base/base.h>

namespace eqNet
{
    /** The base class for all networked objects. */
    class Base
    {
    public:
        /** 
         * Returns the identifier of the object.
         * 
         * @return the object identifier
         */
        uint getID(){ return _id; }
 
    protected:
        /** 
         * Constructs a new object.
         * 
         * @param id the object identifier.
         */
        Base( const uint id ) : _id(id) {}

    private:
        /** The identifier. */
        uint _id;
    };
}

#endif // EQNET_BASE_H
