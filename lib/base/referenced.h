
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include <eq/base/base.h>     // for EQ_EXPORT
#include <eq/base/debug.h>    // for EQ_ERROR
#include <eq/base/spinLock.h> // member

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
        void ref()   
        { 
#ifndef NDEBUG
            EQASSERT( !_hasBeenDeleted );
#endif
            _mutex.set(); ++_refCount; _mutex.unset();
        }
        void unref() 
            { 
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
#endif
                EQASSERT( _refCount > 0 ); 
                _mutex.set();
                --_refCount;
                const bool deleteMe = (_refCount==0);
                _mutex.unset();
                if( deleteMe )
                    delete this;
            }

        int  getRefCount() const { return _refCount; }

    protected:
        Referenced()
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        Referenced( const Referenced& from ) 
            : _refCount(0)
#ifndef NDEBUG
            , _hasBeenDeleted( false )
#endif
            {}

        virtual ~Referenced() 
            {
#ifndef NDEBUG
                _hasBeenDeleted = true;
#endif
                if( _refCount!=0 ) 
                {
                    EQERROR << "Deleting object with ref count " << _refCount
                            << std::endl;
                }
                EQASSERT( _refCount == 0 );
            }

        uint32_t _refCount;
        SpinLock _mutex;
#ifndef NDEBUG
        bool _hasBeenDeleted;
#endif
    };
}

#endif //EQBASE_REFERENCED_H
