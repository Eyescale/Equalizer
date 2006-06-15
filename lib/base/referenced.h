
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include "base.h"
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
        void unref() 
            { 
                EQASSERT( _refCount > 0 ); 
                --_refCount;
                if( _refCount==0 )
                    delete this;
            }

        int  getRefCount() const { return _refCount; }

    protected:
        Referenced() : _refCount(0) {}
        Referenced( const Referenced& from ) : _refCount(0) {}
        virtual ~Referenced() 
            {
                if( _refCount!=0 ) 
                {
                    EQERROR << "Deleting object with ref count " << _refCount
                            << std::endl;
                }
                EQASSERT( _refCount == 0 );
            }

        int _refCount;
    };
}

#endif //EQBASE_REFERENCED_H
