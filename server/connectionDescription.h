
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_CONNECTION_DESCRIPTION_H
#define EQSERVER_CONNECTION_DESCRIPTION_H

#include <eq/net/connectionDescription.h>

namespace eq
{
namespace server
{
    class ConnectionDescription : public net::ConnectionDescription
    {
    public:
        ConnectionDescription();

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array init in connectionDescription.cpp
        enum SAttribute
        {
            SATTR_HOSTNAME,
            SATTR_LAUNCH_COMMAND,
            SATTR_ALL
        };

        enum CAttribute
        {
            CATTR_LAUNCH_COMMAND_QUOTE,
            CATTR_ALL
        };

        enum IAttribute
        {
            IATTR_TYPE,
            IATTR_TCPIP_PORT,
            IATTR_LAUNCH_TIMEOUT,
            IATTR_ALL
        };
        //*}

        static const std::string&  getSAttributeString( const SAttribute attr )
            { return _sAttributeStrings[attr]; }
        static const std::string&  getCAttributeString( const CAttribute attr )
            { return _cAttributeStrings[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
    protected:
        virtual ~ConnectionDescription() {}

    private:
        /** String representation of string attributes. */
        static std::string _sAttributeStrings[SATTR_ALL];
        /** String representation of character attributes. */
        static std::string _cAttributeStrings[CATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];
    };

    std::ostream& operator << ( std::ostream& os, 
                                const net::ConnectionDescription* desc );
}
}
#endif // EQNET_CONNECTION_DESCRIPTION_H
