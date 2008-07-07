
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTDATAOSTREAM_H
#define EQNET_OBJECTDATAOSTREAM_H

#include <eq/net/dataOStream.h>   // base class
#include <eq/net/object.h>        // used in inline constructor

namespace eq
{
namespace net
{
    class Object;

    /**
     * The DataOStream for object data.
     */
    class EQ_EXPORT ObjectDataOStream : public DataOStream
    {
    public:
        ObjectDataOStream( const Object* object )
            : _object( object ), _version( Object::VERSION_NONE )
            , _instanceID( EQ_ID_ANY )
        {}

        virtual ~ObjectDataOStream(){}
 
        void setVersion( const uint32_t version ) { _version = version; }
        uint32_t getVersion() const { return _version; }

        void setInstanceID( const uint32_t instanceID )
            { _instanceID = instanceID; }
        uint32_t getInstanceID() const { return _instanceID; }

    protected:
        const Object* _object;
        uint32_t      _version;
        uint32_t      _instanceID;
    };
}
}
#endif //EQNET_OBJECTDATAOSTREAM_H
