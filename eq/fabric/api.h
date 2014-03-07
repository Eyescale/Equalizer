/* Copyright (c) 2010, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2010-2014, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_API_H
#define EQFABRIC_API_H

#include <lunchbox/api.h>

#if defined(EQUALIZERFABRIC_STATIC)
#  define EQFABRIC_API
#  define EQFABRIC_INL
#elif defined(EQUALIZERFABRIC_SHARED)
#  define EQFABRIC_API LUNCHBOX_DLLEXPORT
#else
#  define EQFABRIC_API LUNCHBOX_DLLIMPORT
#endif

#if defined(EQUALIZER_SHARED) || defined(EQUALIZERSERVER_SHARED) || \
    defined(EQUALIZERFABRIC_SHARED) || defined(EQUALIZERADMIN_SHARED)
#  define EQFABRIC_INL LB_DLLEXPORT
#elif !defined(EQFABRIC_INL)
#  define EQFABRIC_INL LB_DLLIMPORT
#endif

#endif //EQFABRIC_API_H
