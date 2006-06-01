
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

    protected:
        virtual ~ConnectionDescription() {}

    private:
    };

    std::ostream& operator << ( std::ostream& os, 
                                const eqNet::ConnectionDescription* desc );
};

#endif // EQNET_CONNECTION_DESCRIPTION_H
