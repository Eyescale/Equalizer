
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODESTATISTICS_H
#define EQ_NODESTATISTICS_H

#include <eq/client/configEvent.h> // member

namespace eq
{
    class Node;

    /**
     * Holds one statistics event, used for profiling.
     */
    class NodeStatistics
    {
    public:
        NodeStatistics( const Statistic::Type type, Node* node, 
                        const uint32_t frameNumber );
        ~NodeStatistics();

        ConfigEvent event;

    private:
        Node* const _node;
    };
}

#endif // EQ_NODESTATISTICS_H
