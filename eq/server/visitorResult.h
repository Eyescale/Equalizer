
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_VISITORRESULT_H
#define EQSERVER_VISITORRESULT_H

#include <lunchbox/visitorResult.h> // 'base' class

namespace eq
{
namespace server
{
     /** The result code from any visit operation. */
    typedef lunchbox::VisitorResult VisitorResult;

    /** @cond IGNORE */
    using lunchbox::TRAVERSE_CONTINUE;
    using lunchbox::TRAVERSE_TERMINATE;
    using lunchbox::TRAVERSE_PRUNE;
    /** @endcond */
}
}
#endif // EQSERVER_VISITORRESULT_H
