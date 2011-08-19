
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

#ifndef EQADMIN_VISITORRESULT_H
#define EQADMIN_VISITORRESULT_H

#include <eq/fabric/visitorResult.h> // 'base' class

namespace eq
{
namespace admin
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
#endif // EQADMIN_VISITORRESULT_H
