
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

#ifndef EQADMIN_PIPE_H
#define EQADMIN_PIPE_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/pipe.h>       // base class

namespace eq
{
namespace admin
{
    class Node;
    class Window;

    class Pipe : public fabric::Pipe< Node, Pipe, Window, PipeVisitor >
    {
    public:
        /** Construct a new pipe. @version 1.0 */
        EQADMIN_EXPORT Pipe( Node* parent );

        /** Destruct a pipe. @version 1.0 */
        EQADMIN_EXPORT virtual ~Pipe();

        /** @name Data Access. */
        //@{
        ServerPtr getServer(); //!< @return the server node
        ClientPtr getClient(); //!< @return the local client node
        EQADMIN_EXPORT Config* getConfig();   //!< @return the parent configuration
        const Config* getConfig() const; //!< @return the parent configuration

        co::CommandQueue* getMainThreadQueue(); //!< @internal
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQADMIN_PIPE_H

