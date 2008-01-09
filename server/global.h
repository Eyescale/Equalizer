
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_GLOBAL_H
#define EQS_GLOBAL_H

#include "channel.h"
#include "compound.h"
#include "connectionDescription.h"
#include "node.h"
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

        int32_t     _pipeIAttributes[Pipe::IATTR_ALL];

        int32_t     _windowIAttributes[eq::Window::IATTR_ALL];

        int32_t     _channelIAttributes[eq::Channel::IATTR_ALL];
        
        int32_t     _compoundIAttributes[Compound::IATTR_ALL];

        void _setupDefaults();
        void _readEnvironment();

        friend std::ostream& operator << ( std::ostream&, const Global* );
    };

    std::ostream& operator << ( std::ostream& os, const Global* global );
};
#endif // EQS_GLOBAL_H
