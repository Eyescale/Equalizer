
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include <eq/base/base.h>
#include <eq/base/lock.h>
#include <eq/base/log.h>

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
        // TODO: use fast mutex (futex, atomic-op based spinlock or similar)

        void ref()   { _mutex.set(); _refCount++; _mutex.unset(); }
        void unref() 
            { 
                _mutex.set();
                EQASSERT( _refCount > 0 ); 
                --_refCount;
                if( _refCount==0 )
                    delete this;
                _mutex.unset();
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

        int  _refCount;
        Lock _mutex;
    };
}

#endif //EQBASE_REFERENCED_H
