
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

#ifndef EQADMIN_SEGMENT_H
#define EQADMIN_SEGMENT_H

#include <eq/admin/types.h>         // typedefs
#include <eq/fabric/segment.h>      // base class

namespace eq
{
namespace admin
{
class Segment : public fabric::Segment< Canvas, Segment, Channel >
{
public:
    /** Construct a new segment. @version 1.0 */
    EQADMIN_API explicit Segment( Canvas* parent );

    /** Destruct a segment. @version 1.0 */
    EQADMIN_API virtual ~Segment();

    /** @name Data Access */
    //@{
    /** @return the config of this segment. */
    EQADMIN_API Config* getConfig();

    /** @return the config of this segment. */
    EQADMIN_API const Config* getConfig() const;

    /** @return the Server of this segment. */
    EQADMIN_API ServerPtr getServer();
    //@}
};
}
}

#endif // EQADMIN_SEGMENT_H
