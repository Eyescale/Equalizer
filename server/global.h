
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_GLOBAL_H
#define EQS_GLOBAL_H

#include "node.h"

namespace eqs
{
    /**
     * The global default attributes.
     */
    class Global
    {
    public:
        static Global* instance();

        void setNodeSAttribute( const Node::SAttribute attr, 
                                const std::string& value )
            { _nodeSAttributes[attr] = value; }
        const std::string& getNodeSAttribute( const Node::SAttribute attr) const
            { return _nodeSAttributes[attr]; }

        void setNodeIAttribute( const Node::IAttribute attr, const int value )
            { _nodeIAttributes[attr] = value; }
        int getNodeIAttribute( const Node::IAttribute attr ) const
            { return _nodeIAttributes[attr]; }

    private:
        Global(){}
        
        std::string _nodeSAttributes[Node::SATTR_ALL];
        int         _nodeIAttributes[Node::IATTR_ALL];
    };

    std::ostream& operator << ( std::ostream& os, const Global* global );
};
#endif // EQS_GLOBAL_H
