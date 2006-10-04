
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_GLOBAL_H
#define EQS_GLOBAL_H

#include "node.h"
#include "connectionDescription.h"
#include "window.h"

#include <eq/client/global.h>

namespace eqs
{
    /**
     * The global default attributes.
     */
    class Global
    {
    public:
        static Global* instance();

        /**
         * @name Node Attributes.
         */
        void setNodeSAttribute( const Node::SAttribute attr, 
                                const std::string& value )
            { _nodeSAttributes[attr] = value; }
        const std::string& getNodeSAttribute( const Node::SAttribute attr) const
            { return _nodeSAttributes[attr]; }

        void setNodeIAttribute(const Node::IAttribute attr, const int32_t value)
            { _nodeIAttributes[attr] = value; }
        int32_t getNodeIAttribute( const Node::IAttribute attr ) const
            { return _nodeIAttributes[attr]; }

        /**
         * @name Connection (Description) Attributes.
         */
        void setConnectionSAttribute( const ConnectionDescription::SAttribute 
                                      attr, const std::string& value )
            { _connectionSAttributes[attr] = value; }
        const std::string& getConnectionSAttribute(
            const ConnectionDescription::SAttribute attr) const
            { return _connectionSAttributes[attr]; }

        void setConnectionIAttribute( const ConnectionDescription::IAttribute 
                                      attr, const int32_t value)
            { _connectionIAttributes[attr] = value; }
        int32_t getConnectionIAttribute( 
            const ConnectionDescription::IAttribute attr ) const
            { return _connectionIAttributes[attr]; }
            
        /**
         * @name Window Attributes.
         */
        void setWindowIAttribute( const eq::Window::IAttribute attr,
                                  const uint32_t value )
            { _windowIAttributes[attr] = value; }
        int32_t getWindowIAttribute( const eq::Window::IAttribute attr ) const
            { return _windowIAttributes[attr]; }

        /**
         * @name Compound Attributes.
         */  
        void setConfigFAttribute( const Config::FAttribute attr,
                                    const float value )
            { _configFAttributes[attr] = value; }
        float getConfigFAttribute( const Config::FAttribute attr ) const
            { return _configFAttributes[attr]; }
    private:
        Global();
        
        std::string _nodeSAttributes[Node::SATTR_ALL];
        int32_t     _nodeIAttributes[Node::IATTR_ALL];

        std::string _connectionSAttributes[ConnectionDescription::SATTR_ALL];
        int32_t     _connectionIAttributes[ConnectionDescription::IATTR_ALL];
        
        int32_t     _windowIAttributes[eq::Window::IATTR_ALL];
        
        float       _configFAttributes[Config::FATTR_ALL];

        friend std::ostream& operator << ( std::ostream&, const Global* );
    };

    std::ostream& operator << ( std::ostream& os, const Global* global );
};
#endif // EQS_GLOBAL_H
