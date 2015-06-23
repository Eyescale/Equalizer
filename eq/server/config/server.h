
/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.h>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_CONFIG_SERVER_H
#define EQSERVER_CONFIG_SERVER_H

#include <eq/server/api.h>
#include "../types.h"

namespace eq
{
namespace server
{
namespace config
{

class Server
{
public:
    static EQSERVER_API Config* configure( ServerPtr server,
                                           const std::string& session,
                                           const fabric::ConfigParams& params );
    static EQSERVER_API void release( Config* config );
};

}
}
}
#endif // EQSERVER_CONFIG_SERVER_H
