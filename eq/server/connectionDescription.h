
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_CONNECTION_DESCRIPTION_H
#define EQSERVER_CONNECTION_DESCRIPTION_H

#include <eq/server/api.h>
#include <co/connectionDescription.h>

namespace eq
{
namespace server
{
class ConnectionDescription : public co::ConnectionDescription
{
public:
    EQSERVER_API ConnectionDescription();

    /** @name Attributes */
    //@{
    // Note: also update string array init in connectionDescription.cpp
    /** String attributes */
    enum SAttribute
    {
        SATTR_HOSTNAME,
        SATTR_FILENAME,
        SATTR_FILL1,
        SATTR_FILL2,
        SATTR_ALL
    };

    /** Integer attributes */
    enum IAttribute
    {
        IATTR_TYPE,
        IATTR_PORT,
        IATTR_BANDWIDTH,
        IATTR_FILL1,
        IATTR_FILL2,
        IATTR_ALL
    };
    //@}

    static const std::string& getSAttributeString( const SAttribute attr );
    static const std::string& getIAttributeString( const IAttribute attr );

protected:
    virtual ~ConnectionDescription() {}
};
}
}
#endif // EQSERVER_CONNECTION_DESCRIPTION_H
