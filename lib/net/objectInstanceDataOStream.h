
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTINSTANCEDATAOSTREAM_H
#define EQNET_OBJECTINSTANCEDATAOSTREAM_H

#include "objectDataOStream.h"   // base class

namespace eqNet
{
    class Object;

    /**
     * The DataOStream for object instance data.
     */
    class EQ_EXPORT ObjectInstanceDataOStream : public ObjectDataOStream
    {
    public:
        ObjectInstanceDataOStream( const Object* object );
        virtual ~ObjectInstanceDataOStream();
 
    protected:
        virtual void sendHeader( const void* buffer, const uint64_t size )
            { sendBuffer( buffer, size ); }
        virtual void sendBuffer( const void* buffer, const uint64_t size );
        virtual void sendFooter( const void* buffer, const uint64_t size );
        virtual void sendSingle( const void* buffer, const uint64_t size )
            { sendFooter( buffer, size ); }
    };
}
#endif //EQNET_OBJECTINSTANCEDATAOSTREAM_H
