
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTINSTANCEDATAISTREAM_H
#define EQNET_OBJECTINSTANCEDATAISTREAM_H

#include "objectDataIStream.h"   // base class

#include <deque>

namespace eqNet
{
    class Command;

    /**
     * The DataIStream for object instance data.
     */
    class EQ_EXPORT ObjectInstanceDataIStream : public ObjectDataIStream
    {
    public:
        ObjectInstanceDataIStream();
        virtual ~ObjectInstanceDataIStream();

    protected:
        virtual bool getNextBuffer( const uint8_t** buffer, uint64_t* size );

    private:
        uint32_t _sequence;
    };
}
#endif //EQNET_OBJECTINSTANCEDATAISTREAM_H
