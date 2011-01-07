
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

#ifndef EQ_LAYOUT_H
#define EQ_LAYOUT_H

#include <eq/api.h>
#include <eq/types.h>
#include <eq/visitorResult.h>  // enum

#include <eq/fabric/layout.h>         // base class

namespace eq
{
    class Config;
    class Observer;

    /**
     * A layout groups one or more View, logically belonging together.
     *
     * A layout belongs to one or more Canvas. Currently, the layout assignment
     * can be changed at run-time by the application, out of a pre-defined set
     * of layouts for each canvas.
     * 
     * The intersection between views and segments defines which destination
     * channels are available. Neither the views nor the segments have to
     * cover the full layout or canvas, respectively. These channels are
     * typically used as a destination Channel in a compound. They are
     * automatically created when the configuration is loaded.
     *
     * @sa fabric::Layout
     */
    class Layout : public fabric::Layout< Config, Layout, View >
    {
    public:
        /** Construct a new layout. @version 1.0 */
        EQ_API Layout( Config* parent );

        /** Destruct a layout. @version 1.0 */
        EQ_API virtual ~Layout();

        /** @name Data Access */
        //@{
        /** @return the Server of this layout. @version 1.0 */
        EQ_API ServerPtr getServer();
        //@}

    private:
        struct Private;
        Private* _private; // placeholder for binary-compatible changes
    };
}
#endif // EQ_LAYOUT_H
