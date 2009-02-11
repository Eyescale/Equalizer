
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CHANNELVISITOR_H
#define EQSERVER_CHANNELVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Channel;

    /**
     * A visitor to traverse a non-const channels.
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

    /**
     * A visitor to traverse a const channels.
     */
    class ConstChannelVisitor
    {
    public:
        /** Constructs a new ChannelVisitor. */
        ConstChannelVisitor(){}
        
        /** Destruct the ChannelVisitor */
        virtual ~ConstChannelVisitor(){}

        /** Visit a channel. */
        virtual VisitorResult visit( const Channel* channel )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_CHANNELVISITOR_H
