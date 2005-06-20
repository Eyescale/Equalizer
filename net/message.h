
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_MESSAGE_H
#define EQNET_MESSAGE_H

namespace eqNet
{
    /**
     * Represents a message.
     *
     * For performance reasons, this class currently contains only the Type
     * enum. All message parameters are passed directly to the responding send
     * and recv methods to avoid the necessity to create a Message object.
     *
     * @sa Node::send, Node::recv, Network::send, Network::recv
     */
    class Message {
    public:
        /** The type of the message. */
        enum Type {
        };
    };
};

#endif // EQNET_MESSAGE_H

