
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQBASE_NONCOPYABLE_H
#define EQBASE_NONCOPYABLE_H

#include <eq/base/base.h>
#include <eq/base/lock.h>
#include <eq/base/log.h>

namespace eqBase
{
    /**
     * Base class for nonCopyable objects.
     * @sa RefPtr
     */
    class NonCopyable 
    {
    protected:
        NonCopyable() {}
        virtual ~NonCopyable(){}

    private:
        /** Disable copy constructor. */
        NonCopyable( const NonCopyable& from );
        /** Disable assignment operator. */
        const NonCopyable& operator = ( const NonCopyable& from );
    };
}

#endif //EQBASE_NONCOPYABLE_H
