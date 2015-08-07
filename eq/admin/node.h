
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQADMIN_NODE_H
#define EQADMIN_NODE_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/node.h>       // base class

namespace eq
{
namespace admin
{
class Node : public fabric::Node< Config, Node, Pipe, NodeVisitor >
{
public:
    /** Construct a new node. @version 1.0 */
    EQADMIN_API explicit Node( Config* parent );

    /** Destruct a node. @version 1.0 */
    EQADMIN_API virtual ~Node();

    /** @name Data Access. */
    //@{
    ServerPtr getServer();
    //@}
};
}
}

#endif // EQADMIN_NODE_H
