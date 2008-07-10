
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_CHANNELVISITOR_H
#define EQ_CHANNELVISITOR_H

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

        /** The result of a visit operation. */
        enum Result
        {
            TRAVERSE_CONTINUE,   //!< continue the traversal
            TRAVERSE_TERMINATE,  //!< abort the traversal
            TRAVERSE_PRUNE       //!< do not traverse current entity downwards
        };

        /** Visit a channel. */
        virtual Result visit( Channel* channel )
            { return TRAVERSE_CONTINUE; }
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const ChannelVisitor::Result result )
    {
        switch( result )
        {
            case ChannelVisitor::TRAVERSE_CONTINUE:
                os << "continue";
                break;
            case ChannelVisitor::TRAVERSE_TERMINATE:
                os << "terminate";
                break;
            case ChannelVisitor::TRAVERSE_PRUNE:
                os << "prune";
                break;
            default:
                os << "ERROR";
                break;
        }
        return os;
    }
}
#endif // EQ_CHANNELVISITOR_H
