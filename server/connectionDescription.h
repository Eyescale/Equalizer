
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CONNECTION_DESCRIPTION_H
#define EQS_CONNECTION_DESCRIPTION_H

#include <eq/net/connectionDescription.h>

namespace eqs
{
    class ConnectionDescription : public eqNet::ConnectionDescription
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
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
    protected:
        virtual ~ConnectionDescription() {}

    private:
        /** String representation of string attributes. */
        static std::string _sAttributeStrings[SATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];
    };

    std::ostream& operator << ( std::ostream& os, 
                                const eqNet::ConnectionDescription* desc );
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
