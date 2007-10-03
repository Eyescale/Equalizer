
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLOBAL_H
#define EQ_GLOBAL_H

#include <eq/base/base.h>
#include <eq/base/lock.h> // member
#include <string>

namespace eq
{
    class NodeFactory;

    /** Possible values for some integer attributes */
    enum IAttrValue
    {
        UNDEFINED  = -0xfffffff,
        VERTICAL   = -5,
        QUAD       = -4,
        ANAGLYPH   = -3,
        NICEST     = -2,
        AUTO       = -1,
        OFF        = false,
        ON         = true,
        FASTEST    = ON,
        HORIZONTAL = ON
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

        /** 
         * Global lock for all non-thread-safe Carbon API calls. 
         * Note: this is a nop on non-AGL builds. Do not use unless you know the
         * side effects, i.e., ask on the eq-dev mailing list.
         */
        static void enterCarbon();
        /** Global unlock for all non-thread-safe Carbon API calls */
        static void leaveCarbon();

    private:
		friend EQ_EXPORT bool init( const int argc, char** argv, 
                                    NodeFactory* nodeFactory );
		friend EQ_EXPORT bool exit();
        static NodeFactory* _nodeFactory;

        static std::string  _server;

        static eqBase::Lock _carbonLock;
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const IAttrValue value );
}

#endif // EQ_GLOBAL_H

