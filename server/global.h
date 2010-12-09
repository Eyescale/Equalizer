
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2008-2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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
#include "channel.h"                 // nested enum
#include "config.h"                  // nested enum
#include "connectionDescription.h"   // nested enum
#include "pipe.h"                    // nested enum
#include "node.h"                    // nested enum
#include "window.h"                  // nested enum


namespace eq
{
namespace server
{
    /**
     * The global default attributes.
     */
    class Global
    {
    public:
        EQSERVER_EXPORT static Global* instance();

        /** De-allocate the global instance. */
        EQSERVER_EXPORT static void clear();

        /** @name Connection (Description) Attributes. */
        //@{
        void setConnectionSAttribute( const ConnectionDescription::SAttribute 
                                      attr, const std::string& value )
            { _connectionSAttributes[attr] = value; }
        const std::string& getConnectionSAttribute(
            const ConnectionDescription::SAttribute attr ) const
            { return _connectionSAttributes[attr]; }

        void setConnectionIAttribute( const ConnectionDescription::IAttribute 
                                      attr, const int32_t value)
            { _connectionIAttributes[attr] = value; }
        int32_t getConnectionIAttribute( 
            const ConnectionDescription::IAttribute attr ) const
            { return _connectionIAttributes[attr]; }
        //@}
            
        /** @name Config Attributes. */  
        //@{
        void setConfigFAttribute( const Config::FAttribute attr,
                                  const float value )
            { _configFAttributes[attr] = value; }
        float getConfigFAttribute( const Config::FAttribute attr ) const
            { return _configFAttributes[attr]; }

        void setConfigIAttribute( const Config::IAttribute attr,
                                  const int32_t value )
            { _configIAttributes[attr] = value; }
        int32_t getConfigIAttribute( const Config::IAttribute attr ) const
            { return _configIAttributes[attr]; }
        //@}

        /** @name Node Attributes. */  
        //@{
        void setNodeSAttribute( const Node::SAttribute attr,
                                const std::string& value )
            { _nodeSAttributes[attr] = value; }
        const std::string& getNodeSAttribute( const Node::SAttribute attr) const
            { return _nodeSAttributes[attr]; }

        void setNodeCAttribute( const Node::CAttribute attr, const char value )
            { _nodeCAttributes[attr] = value; }
        char getNodeCAttribute( const Node::CAttribute attr ) const
            { return _nodeCAttributes[attr]; }

        void setNodeIAttribute( const Node::IAttribute attr,
                                const int32_t value )
            { _nodeIAttributes[attr] = value; }
        int32_t getNodeIAttribute( const Node::IAttribute attr ) const
            { return _nodeIAttributes[attr]; }
        //@}

        /** @name Pipe Attributes. */
        //@{
        void setPipeIAttribute( const Pipe::IAttribute attr,
                                  const int32_t value )
            { _pipeIAttributes[attr] = value; }
        int32_t getPipeIAttribute( const Pipe::IAttribute attr ) const
            { return _pipeIAttributes[attr]; }
        //@}

        /** @name Window Attributes. */
        //@{
        void setWindowIAttribute( const Window::IAttribute attr,
                                  const int32_t value )
            { _windowIAttributes[attr] = value; }
        int32_t getWindowIAttribute( const Window::IAttribute attr ) const
            { return _windowIAttributes[attr]; }
        //@}

        /** @name Channel Attributes. */
        //@{
        void setChannelIAttribute( const Channel::IAttribute attr,
                                   const int32_t value )
            { _channelIAttributes[attr] = value; }
        int32_t getChannelIAttribute( const Channel::IAttribute attr ) const
            { return _channelIAttributes[attr]; }
        //@}

        /** @name Compound Attributes. */  
        //@{
        void setCompoundIAttribute( const Compound::IAttribute attr,
                                    const int32_t value )
            { _compoundIAttributes[attr] = value; }
        int32_t getCompoundIAttribute( const Compound::IAttribute attr ) const
            { return _compoundIAttributes[attr]; }
        //@}

    private:
        Global();
        
        std::string _connectionSAttributes[ConnectionDescription::SATTR_ALL];
        int32_t     _connectionIAttributes[ConnectionDescription::IATTR_ALL];
        
        float       _configFAttributes[Config::FATTR_ALL];
        int32_t     _configIAttributes[Config::IATTR_ALL];

        std::string _nodeSAttributes[Node::SATTR_ALL];
        char        _nodeCAttributes[Node::CATTR_ALL];
        int32_t     _nodeIAttributes[Node::IATTR_ALL];

        int32_t     _pipeIAttributes[Pipe::IATTR_ALL];

        int32_t     _windowIAttributes[Window::IATTR_ALL];

        int32_t     _channelIAttributes[Channel::IATTR_ALL];
        
        int32_t     _compoundIAttributes[Compound::IATTR_ALL];

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        void _setupDefaults();
        void _readEnvironment();

        friend EQSERVER_EXPORT std::ostream& operator << ( std::ostream&,
                                                           const Global* );
    };

    EQSERVER_EXPORT std::ostream& operator << ( std::ostream&, const Global* );
}
}
#endif // EQSERVER_GLOBAL_H
