
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQSERVER_GLOBAL_H
#define EQSERVER_GLOBAL_H

#include "compound.h"                // nested enum
#include "config.h"                  // nested enum
#include "connectionDescription.h"   // nested enum
#include "pipe.h"                    // nested enum

//#include <eq/client/global.h>
#include <eq/client/node.h>      // nested enum
#include <eq/client/channel.h>   // nested enum
#include <eq/client/window.h>    // nested enum

namespace eq
{
namespace server
{
    /**
     * The global default attributes.
     */
    class EQSERVER_EXPORT Global
    {
    public:
        static Global* instance();

        /** De-allocate the global instance. */
        static void clear();

        /**
         * @name Connection (Description) Attributes.
         */
        void setConnectionSAttribute( const ConnectionDescription::SAttribute 
                                      attr, const std::string& value )
            { _connectionSAttributes[attr] = value; }
        const std::string& getConnectionSAttribute(
            const ConnectionDescription::SAttribute attr ) const
            { return _connectionSAttributes[attr]; }

        void setConnectionCAttribute( const ConnectionDescription::CAttribute 
                                      attr, const char value )
            { _connectionCAttributes[attr] = value; }
        char getConnectionCAttribute(
            const ConnectionDescription::CAttribute attr ) const
            { return _connectionCAttributes[attr]; }

        void setConnectionIAttribute( const ConnectionDescription::IAttribute 
                                      attr, const int32_t value)
            { _connectionIAttributes[attr] = value; }
        int32_t getConnectionIAttribute( 
            const ConnectionDescription::IAttribute attr ) const
            { return _connectionIAttributes[attr]; }
            
        /**
         * @name Config Attributes.
         */  
        void setConfigFAttribute( const Config::FAttribute attr,
                                    const float value )
            { _configFAttributes[attr] = value; }
        float getConfigFAttribute( const Config::FAttribute attr ) const
            { return _configFAttributes[attr]; }

        /**
         * @name Node Attributes.
         */  
        void setNodeIAttribute( const eq::Node::IAttribute attr,
                                    const int32_t value )
            { _nodeIAttributes[attr] = value; }
        int32_t getNodeIAttribute( const eq::Node::IAttribute attr ) const
            { return _nodeIAttributes[attr]; }

        /**
         * @name Pipe Attributes.
         */
        void setPipeIAttribute( const Pipe::IAttribute attr,
                                  const int32_t value )
            { _pipeIAttributes[attr] = value; }
        int32_t getPipeIAttribute( const Pipe::IAttribute attr ) const
            { return _pipeIAttributes[attr]; }

        /**
         * @name Window Attributes.
         */
        void setWindowIAttribute( const eq::Window::IAttribute attr,
                                  const int32_t value )
            { _windowIAttributes[attr] = value; }
        int32_t getWindowIAttribute( const eq::Window::IAttribute attr ) const
            { return _windowIAttributes[attr]; }

        /**
         * @name Channel Attributes.
         */
        void setChannelIAttribute( const eq::Channel::IAttribute attr,
                                  const int32_t value )
            { _channelIAttributes[attr] = value; }
        int32_t getChannelIAttribute( const eq::Channel::IAttribute attr ) const
            { return _channelIAttributes[attr]; }

        /**
         * @name Compound Attributes.
         */  
        void setCompoundIAttribute( const Compound::IAttribute attr,
                                    const int32_t value )
            { _compoundIAttributes[attr] = value; }
        int32_t getCompoundIAttribute( const Compound::IAttribute attr ) const
            { return _compoundIAttributes[attr]; }

    private:
        Global();
        
        std::string _connectionSAttributes[ConnectionDescription::SATTR_ALL];
        char        _connectionCAttributes[ConnectionDescription::CATTR_ALL];
        int32_t     _connectionIAttributes[ConnectionDescription::IATTR_ALL];
        
        float       _configFAttributes[Config::FATTR_ALL];

        int32_t     _nodeIAttributes[eq::Node::IATTR_ALL];

        int32_t     _pipeIAttributes[Pipe::IATTR_ALL];

        int32_t     _windowIAttributes[eq::Window::IATTR_ALL];

        int32_t     _channelIAttributes[eq::Channel::IATTR_ALL];
        
        int32_t     _compoundIAttributes[Compound::IATTR_ALL];

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        void _setupDefaults();
        void _readEnvironment();

        friend std::ostream& operator << ( std::ostream&, const Global* );
    };

    std::ostream& operator << ( std::ostream& os, const Global* global );
}
}
#endif // EQSERVER_GLOBAL_H
