
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <string>

namespace eq
{
    class NodeFactory;

    /** Possible values for some integer attributes */
    enum IAttrValue
    {
        UNDEFINED = -2,
        AUTO      = -1,
        OFF       = false,
        ON        = true,
    };

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

        /** 
         * Sets the default Equalizer server.
         * 
         * @param server the default server.
         */
        static void setServer( const std::string& server )
            { _server = server; }

        /** 
         * Gets the default Equalizer server.
         * 
         * @return the default server.
         */
        static const std::string& getServer() { return _server; }

    private:
        static NodeFactory* _nodeFactory;
        static std::string  _server;
    };

    std::ostream& operator << ( std::ostream& os, const IAttrValue value );
}

#endif // EQ_GLOBAL_H

