
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

namespace eq
{
    class NodeFactory;

    /** 
     * Global parameter handling for the Equalizer client library. 
     */
    class Global
    {
    public:
        /** 
         * Gets the node factory.
         * 
         * @return the node factory.
         */
        static NodeFactory* getNodeFactory() { return _nodeFactory; }

    private:
        static NodeFactory* _nodeFactory;
    };
}

#endif // EQ_GLOBAL_H

