
/* Copyright (c) 2011-2016, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@eyescale.ch>
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

namespace eq
{
/** Filtering algorithm to applied during zoom operations. */
enum ZoomFilter
{
    FILTER_NEAREST = 0x2600, //!< GL_NEAREST
    FILTER_LINEAR  = 0x2601  //!< GL_LINEAR
};
}
#endif // EQ_ZOOMFILTER_H
