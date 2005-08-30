
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include "log.h"

namespace eqBase
{
    /**
     * Base class for referenced objects.
     * @sa RefPtr
     */
    class Referenced 
    {
    public:
        // TODO: optional thread-safety

        void ref()   { _refCount++; }
        void unref() { if( (_refCount--)==0 ) delete this; }

        int  getRefCount() const { return _refCount; }

    protected:
        Referenced() : _refCount(0) {}
        virtual ~Referenced() 
            {
                if( _refCount!=0 ) 
                    ERROR << "Deleting object with ref count " << _refCount
                          << std::endl;
            }

        int _refCount;
    };
}

#endif //EQBASE_REFERENCED_H
