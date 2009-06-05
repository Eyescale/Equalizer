
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_VISITORRESULT_H
#define EQ_VISITORRESULT_H

#include <iostream>

namespace eq
{
     /** The result code from any visit operation. */
    enum VisitorResult
    {
        TRAVERSE_CONTINUE,   //!< continue the traversal
        TRAVERSE_TERMINATE,  //!< abort the traversal
        TRAVERSE_PRUNE       //!< do not traverse current entity downwards
    };

    inline std::ostream& operator << ( std::ostream& os, 
                                       const VisitorResult result )
    {
        switch( result )
        {
            case TRAVERSE_CONTINUE:
                os << "continue";
                break;
            case TRAVERSE_TERMINATE:
                os << "terminate";
                break;
            case TRAVERSE_PRUNE:
                os << "prune";
                break;
            default:
                os << "ERROR";
                break;
        }
        return os;
    }
}
#endif // EQ_VISITORRESULT_H
