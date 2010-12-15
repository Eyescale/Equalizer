
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

#ifndef EQADMIN_WINDOW_H
#define EQADMIN_WINDOW_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/window.h>       // base class

namespace eq
{
namespace admin
{
    class Window : public fabric::Window< Pipe, Window, Channel >
    {
    public:
        /** Construct a new window. @version 1.0 */
        EQADMIN_EXPORT Window( Pipe* parent );

        /** Destruct this window. @version 1.0 */
        EQADMIN_EXPORT virtual ~Window();

        /** @name Data Access */
        //@{
        /** @return the Config of this window. */
        EQADMIN_EXPORT const Config* getConfig() const;

        /** @return the Server of this window. */
        EQADMIN_EXPORT ServerPtr getServer();

        /** @return the Config of this window. */
        EQADMIN_EXPORT Config* getConfig();
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQADMIN_WINDOW_H

