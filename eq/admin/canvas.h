
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQADMIN_CANVAS_H
#define EQADMIN_CANVAS_H

#include <eq/admin/types.h>   // typedefs
#include <eq/fabric/canvas.h> // base class

namespace eq
{
namespace admin
{
    class Segment;

    class Canvas : public fabric::Canvas< Config, Canvas, Segment, Layout >
    {
    public:
        /** Construct a new canvas. @version 1.0 */
        EQADMIN_API Canvas( Config* parent );

        /** Destruct this canvas. @version 1.0 */
        EQADMIN_API virtual ~Canvas();

        /** @name Data Access */
        //@{
        /** @return the Server of this canvas. @version 1.0 */
        EQADMIN_API ServerPtr getServer();
        //@}

    private:
        struct Private;
        Private* _private; // placeholder for binary-compatible changes
    };
}
}

#endif // EQADMIN_CANVAS_H

