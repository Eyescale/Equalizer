
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

#ifndef EQSERVER_VISITORRESULT_H
#define EQSERVER_VISITORRESULT_H

#include <eq/fabric/visitorResult.h> // 'base' class

namespace eq
{
namespace server
{
     /** The result code from any visit operation. */
    typedef fabric::VisitorResult VisitorResult;

    /** @cond IGNORE */
    using fabric::TRAVERSE_CONTINUE;
    using fabric::TRAVERSE_TERMINATE;
    using fabric::TRAVERSE_PRUNE;
    /** @endcond */
}
}
#endif // EQSERVER_VISITORRESULT_H
