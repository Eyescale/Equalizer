
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNELVISITOR_H
#define EQ_CHANNELVISITOR_H

#include <eq/client/visitorResult.h>  // enum

#include <iostream>

namespace eq
{
    class Channel;

    /**
     * A visitor to traverse a non-const channels
     */
    class ChannelVisitor
    {
    public:
        /** Constructs a new ChannelVisitor. */
        ChannelVisitor(){}
        
        /** Destruct the ChannelVisitor */
        virtual ~ChannelVisitor(){}

        /** Visit a channel. */
        virtual VisitorResult visit( Channel* channel )
            { return TRAVERSE_CONTINUE; }
    };

}
#endif // EQ_CHANNELVISITOR_H
