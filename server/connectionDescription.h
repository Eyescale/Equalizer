
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
        EQSERVER_EXPORT ConnectionDescription();

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array init in connectionDescription.cpp
        enum SAttribute
        {
            SATTR_HOSTNAME,
            SATTR_LAUNCH_COMMAND,
            SATTR_FILL1,
            SATTR_FILL2,
            SATTR_ALL
        };

        enum CAttribute
        {
            CATTR_LAUNCH_COMMAND_QUOTE,
            CATTR_FILL1,
            CATTR_FILL2,
            CATTR_ALL
        };

        enum IAttribute
        {
            IATTR_TYPE,
            IATTR_TCPIP_PORT,
            IATTR_LAUNCH_TIMEOUT,
            IATTR_BANDWIDTH,
            IATTR_FILL1,
            IATTR_FILL2,
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

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };
    };

    std::ostream& operator << ( std::ostream&, const ConnectionDescription* );
}
}
#endif // EQNET_CONNECTION_DESCRIPTION_H
