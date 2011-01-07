
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

#ifndef EQADMIN_VIEW_H
#define EQADMIN_VIEW_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/view.h>       // base class

namespace eq
{
namespace admin
{
    class View : public fabric::View< Layout, View, Observer >
    {
    public:
        /** Construct a new view. @version 1.0 */
        EQADMIN_EXPORT View( Layout* parent );

        /** Destruct this view. @version 1.0 */
        EQADMIN_EXPORT virtual ~View();

        /** @name Data Access. */
        //@{
        /** @return the config of this view. @version 1.0 */
        EQADMIN_EXPORT Config* getConfig();

        /** @return the config of this view. @version 1.0 */
        EQADMIN_EXPORT const Config* getConfig() const;

        /** @return the Server of this view. @version 1.0 */
        EQADMIN_EXPORT ServerPtr getServer();
        //@}

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQADMIN_VIEW_H

