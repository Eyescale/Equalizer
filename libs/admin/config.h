
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

#ifndef EQADMIN_CONFIG_H
#define EQADMIN_CONFIG_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/config.h>       // base class

namespace eq
{
namespace admin
{
    class Canvas;
    class Layout;
    class Node;
    class Observer;
    struct ConfigEvent;

    class Config : public fabric::Config< Server, Config, Observer, Layout,
                                          Canvas, Node, ConfigVisitor >
    {
    public:
        typedef fabric::Config< Server, Config, Observer, Layout, Canvas, Node,
                                ConfigVisitor > Super;

        /** Construct a new config. @version 1.0 */
        EQADMIN_EXPORT Config( ServerPtr parent );

        /** Destruct a config. @version 1.0 */
        EQADMIN_EXPORT virtual ~Config();

        /** @return the local client node. @version 1.0 */
        EQADMIN_EXPORT ClientPtr getClient();

        /** @return the local client node. @version 1.0 */
        EQADMIN_EXPORT ConstClientPtr getClient() const;

        EQADMIN_EXPORT co::CommandQueue* getMainThreadQueue(); //!< @internal

        /** @internal */
        const Channel* findChannel( const std::string& name ) const
            { return find< Channel >( name ); }

        void output( std::ostream& os ) const {} //!< @internal
        virtual bool mapViewObjects() const { return true; } //!< @internal
        virtual bool mapNodeObjects() const { return true; } //!< @internal

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQADMIN_CONFIG_H

