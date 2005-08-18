
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_MESSAGE_H
#define EQNET_MESSAGE_H

namespace eqNet
{
    /** The type of the message. */
    enum MessageType {
        TYPE_BYTE,
        TYPE_SHORT,
        TYPE_INTEGER,
        TYPE_FLOAT
    };
}

#endif // EQNET_MESSAGE_H

