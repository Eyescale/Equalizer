
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_LAYOUT_H
#define EQSERVER_LAYOUT_H

#include "visitorResult.h" // enum
#include "types.h"

#include <eq/fabric/layout.h> // base class
#include <string>

namespace eq
{
namespace fabric
{
    template< class L, class V, class O > class View;
}
namespace server
{
    /** The layout. @sa eq::Layout */
    class Layout : public fabric::Layout< Config, Layout, View >
    {
    public:
        /** Construct a new Layout. */
        EQSERVER_EXPORT Layout( Config* parent );

         /** Destruct this layout. */
        virtual ~Layout();

        /** @name Data Access */
        //@{
        /** @return the Server of this layout. @version 1.0 */
        ServerPtr getServer();
        //@}

        /** @return true if this layout should be deleted. */
        bool needsDelete() const { return _state == STATE_DELETE; }

        /** @name Operations */
        //@{
        /** 
         * Trigger a layout (de)activation
         *
         * @param canvas The canvas triggering the (de)activation.
         * @param active true to activate, false to deactivate.
         */
        void trigger( const Canvas* canvas, const bool active );

        /** Schedule deletion of this layout. */
        void postDelete();
        //@}

    private:
        enum State
        {
            STATE_ACTIVE = 0,  // next: DELETE
            STATE_DELETE,      // next: destructor
        }
            _state;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };

    std::ostream& operator << ( std::ostream& os, const Layout* layout);
}
}
#endif // EQSERVER_LAYOUT_H
