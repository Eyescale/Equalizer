
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "connectionDescription.h"

#include "global.h"
#include <lunchbox/log.h>
#include <co/connection.h>

namespace eq
{
namespace server
{
namespace
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CONNECTION_") + #attr )

static std::string _sAttributeStrings[ ConnectionDescription::SATTR_ALL ] =
{
    MAKE_ATTR_STRING( SATTR_HOSTNAME ),
    MAKE_ATTR_STRING( SATTR_PIPE_FILENAME ),
    MAKE_ATTR_STRING( SATTR_FILL1 ),
    MAKE_ATTR_STRING( SATTR_FILL2 )
};
static std::string _iAttributeStrings[ ConnectionDescription::IATTR_ALL ] =
{
    MAKE_ATTR_STRING( IATTR_TYPE ),
    MAKE_ATTR_STRING( IATTR_TCPIP_PORT ),
    MAKE_ATTR_STRING( IATTR_BANDWIDTH ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};
}

ConnectionDescription::ConnectionDescription()
{
    const Global* global = Global::instance();

    setHostname( global->getConnectionSAttribute( SATTR_HOSTNAME ));
    type = static_cast< co::ConnectionType >(
        global->getConnectionIAttribute( IATTR_TYPE ));

    bandwidth = global->getConnectionIAttribute( IATTR_BANDWIDTH );

    port = global->getConnectionIAttribute( IATTR_PORT );
    setFilename( global->getConnectionSAttribute( SATTR_FILENAME ));
}

const std::string& ConnectionDescription::getSAttributeString(
    const SAttribute attr )
{
    return _sAttributeStrings[ attr ];
}

const std::string& ConnectionDescription::getIAttributeString( 
    const IAttribute attr )
{
    return _iAttributeStrings[ attr ];
}


}
}
