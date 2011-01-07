
/* Copyright (c) 2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQ_ZOOMFILTER_H
#define EQ_ZOOMFILTER_H

#include <eq/os.h>

namespace eq
{
    /** Filtering algorithm to applied during zoom operations. */
    EQ_API enum ZoomFilter
    {
        FILTER_NEAREST = GL_NEAREST,
        FILTER_LINEAR  = GL_LINEAR 
    };

}
#endif // EQ_ZOOMFILTER_H
