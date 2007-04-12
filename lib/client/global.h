
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
    class NodeFactory;

    /** Possible values for some integer attributes */
    enum IAttrValue
    {
        UNDEFINED = -0xfffffff,
        QUAD      = -4,
        ANAGLYPH  = -3,
        NICEST    = -2,
        AUTO      = -1,
        OFF       = false,
        ON        = true,
        FASTEST   = ON,
    };

    /** 
     * Global parameter handling for the Equalizer client library. 
     */
    class EQ_EXPORT Global
    {
    public:
        /** @return the node factory. */
        static NodeFactory* getNodeFactory() { return _nodeFactory; }

        /** 
         * Set the default Equalizer server.
         * 
         * @param server the default server.
         */
        static void setServer( const std::string& server )
            { _server = server; }

        /** @return the default Equalizer server. */
        static const std::string& getServer() { return _server; }

    private:
		friend EQ_EXPORT bool init( int argc, char** argv, 
                                    NodeFactory* nodeFactory );
		friend EQ_EXPORT bool exit();
        static NodeFactory* _nodeFactory;

        static std::string  _server;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const IAttrValue value );
}

#endif // EQ_GLOBAL_H

