
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <eq/base/lock.h>

namespace eqBase
{
    /**
     * Base class for referenced objects.
     * @sa RefPtr
     */
    class EQ_EXPORT Referenced 
    {
    public:
        // TODO: optional thread-safety
        // TODO: use fast mutex (futex, atomic-op based spinlock or similar)

        void ref()   { _mutex.set(); ++_refCount; _mutex.unset(); }
        void unref() 
            { 
                _mutex.set();
                EQASSERT( _refCount > 0 ); 
                --_refCount;
                const bool deleteMe = (_refCount==0);
                _mutex.unset();
                if( deleteMe )
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

        uint32_t _refCount;
        Lock     _mutex;
    };
}

#endif //EQBASE_REFERENCED_H
