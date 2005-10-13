
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_NODE_H
#define EQS_NODE_H

#include <eq/net/node.h>

#include <vector>

namespace eqs
{
    class Pipe;

    /**
     * The node.
     */
    class Node : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new Node.
         */
        Node() : _used(0) {}

        /** 
         * Adds a new pipe to this node.
         * 
         * @param pipe the pipe.
         */
        void addPipe( Pipe* pipe ){ _pipes.push_back( pipe ); }

        /** 
         * Removes a pipe from this node.
         * 
         * @param pipe the pipe
         * @return <code>true</code> if the pipe was removed, <code>false</code>
         *         otherwise.
         */
        bool removePipe( Pipe* pipe );

        /** 
         * Returns the number of pipes on this node.
         * 
         * @return the number of pipes on this node. 
         */
        uint nPipes() const { return _pipes.size(); }

        /** 
         * Gets a pipe.
         * 
         * @param index the pipe's index. 
         * @return the pipe.
         */
        Pipe* getPipe( const uint index ) const { return _pipes[index]; }

        /** 
         * References this node as being actively used.
         */
        void refUsed(){ _used++; }

        /** 
         * Unreferences this node as being actively used.
         */
        void unrefUsed(){ _used--; }

        /** 
         * Returns if this node is actively used.
         *
         * @return <code>true</code> if this node is actively used,
         *         <code>false</code> if not.
         */
        bool isUsed() const { return (_used!=0); }

    private:
        /** The vector of pipes belonging to this node. */
        std::vector<Pipe*> _pipes;

        /** Number of entitities actively using this channel. */
        uint _used;
    };

    inline std::ostream& operator << ( std::ostream& os, const Node* node )
    {
        if( !node )
        {
            os << "NULL node";
            return os;
        }

        const uint nPipes = node->nPipes();
        os << "node " << (void*)node << " " << nPipes << " pipes";
        for( uint i=0; i<nPipes; i++ )
            os << std::endl << "    " << node->getPipe(i);

        return os;
    }
};
#endif // EQS_NODE_H
