
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTDELTADATAISTREAM_H
#define EQNET_OBJECTDELTADATAISTREAM_H

#include "objectDataIStream.h"   // base class

namespace eqNet
{
    class Command;

    /**
     * The DataIStream for object delta version data.
     */
    class EQ_EXPORT ObjectDeltaDataIStream : public ObjectDataIStream
    {
    public:
        ObjectDeltaDataIStream();
        virtual ~ObjectDeltaDataIStream();

    protected:
        virtual bool getNextBuffer( const void** buffer, uint64_t* size );
    };
}
#endif //EQNET_OBJECTDELTADATAISTREAM_H
