
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <co/connectionDescription.h>

namespace eq
{
namespace server
{
    class ConnectionDescription : public co::ConnectionDescription
    {
    public:
        EQSERVER_EXPORT ConnectionDescription();

    protected:
        virtual ~ConnectionDescription() {}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}
#endif // EQNET_CONNECTION_DESCRIPTION_H
