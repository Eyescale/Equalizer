
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

#ifndef EQADMIN_OBSERVER_H
#define EQADMIN_OBSERVER_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/observer.h>       // base class

namespace eq
{
namespace admin
{
    class Config;

    class Observer : public fabric::Observer< Config, Observer >
    {
    public:
        /** Construct a new observer. @version 1.0 */
        EQADMIN_EXPORT Observer( Config* parent );

        /** Destruct this observer. @version 1.0 */
        EQADMIN_EXPORT virtual ~Observer();

        /** @name Data Access */
        //@{
        /** @return the Server of this observer. @version 1.0 */
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

#endif // EQADMIN_OBSERVER_H

