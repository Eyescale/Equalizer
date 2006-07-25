
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_FRAMEBUFFER_H
#define EQ_FRAMEBUFFER_H

#include <eq/net/object.h>
#include <eq/net/node.h>

namespace eq
{
    /**
     * A frame buffer holds multiple images and is used by frames.
     */
    class FrameBuffer : public eqNet::Object
    {
    public:
        /** Constructs a new frame buffer. */
        FrameBuffer( eqBase::RefPtr<eqNet::Node> master );

        /** Instanciates a frame buffer. */
        FrameBuffer( const void* instanceData );

        virtual ~FrameBuffer(){}

        /** 
         * @name Data Access
         */
        //*{
        //*}

        /** @name Operations */
        //*{
        //*}

    protected:
        /** @sa Object::getInstanceData */
        virtual const void* getInstanceData( uint64_t* size )
            { *size = sizeof( _data ); return &_data; }

        /** @sa Object::init */
        virtual void init( const void* data, const uint64_t dataSize );

        /** @sa Object::pack */
        virtual const void* pack( uint64_t* size )
            {
                *size   = sizeof( _data );
                return &_data;
            }

        /** @sa Object::unpack */
        virtual void unpack( const void* data, const uint64_t size ) 
            { _data = *(Data*)data; }

    private:
        struct Data
        {
            /** The master node, i.e., the one writing the frame. */
            eqNet::NodeID   master;
        }
            _data;

        eqBase::RefPtr<eqNet::Node> _master;

        /* The command handlers. */
        //CommandResult _cmdEnter( Node* node, const Packet* pkg );
    };
}

#endif // EQ_FRAMEBUFFER_H

