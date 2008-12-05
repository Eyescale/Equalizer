
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTDATAISTREAM_H
#define EQNET_OBJECTDATAISTREAM_H

#include <eq/net/dataIStream.h>   // base class
#include <eq/net/object.h>        // nested enum
#include <eq/base/monitor.h>      // member

#include <deque>

namespace eq
{
namespace net
{
    class Command;

    /**
     * The DataIStream for object data.
     */
    class EQ_EXPORT ObjectDataIStream : public DataIStream
    {
    public:
        ObjectDataIStream();
        virtual ~ObjectDataIStream();

        void addDataPacket( const Command& command );

        void setVersion( const uint32_t version ) { _version = version; }
        virtual uint32_t getVersion() const       { return _version.get(); }
        void waitReady() const { _version.waitNE( Object::VERSION_INVALID ); }

        virtual size_t nRemainingBuffers() const  { return _commands.size(); }

        virtual void reset();

    protected:
        const Command* getNextCommand();

    private:
        /** All data command packets for this istream. */
        std::deque< Command* >      _commands;

        /** The last returned, to be released command. */
        Command*                    _lastCommand;

        /** The object version associated with this input stream. */
        base::Monitor< uint32_t > _version;
    };
}
}
#endif //EQNET_OBJECTDATAISTREAM_H
