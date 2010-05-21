
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQADMIN_CONNECTSERVER_H
#define EQADMIN_CONNECTSERVER_H

#include <eq/admin/types.h>
#include <eq/fabric/types.h>

namespace eq
{
namespace admin
{

/** Connect an admin::Server to any Client, e.g., and eq::Client */
EQADMIN_EXPORT bool connectServer( eq::fabric::ClientPtr client,
                                   ServerPtr server );

/** Disconnect a connected admin server. */
EQADMIN_EXPORT bool disconnectServer( eq::fabric::ClientPtr client,
                                      ServerPtr server );

}
}

#endif // EQADMIN_CLIENT_H
