
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_REFERENCED_H
#define EQBASE_REFERENCED_H

#include <eq/base/base.h>     // for EQ_EXPORT
#include <eq/base/debug.h>    // for EQERROR
#include <eq/base/atomic.h>   // member

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
            EQASSERTINFO( !_hasBeenDeleted, typeid( *this ).name( ));
#endif
            ++_refCount;
        }
        void unref() 
            { 
#ifndef NDEBUG
                EQASSERT( !_hasBeenDeleted );
#endif
                EQASSERT( _refCount > 0 ); 
                const bool deleteMe = (--_refCount==0);
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

        Referenced( const Referenced& ) 
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

        Atomic< long > _refCount;
#ifndef NDEBUG
        bool _hasBeenDeleted;
#endif
    };
}

#endif //EQBASE_REFERENCED_H
