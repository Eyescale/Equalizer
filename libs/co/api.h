
/* Copyright (c) 2010, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef CO_API_H
#define CO_API_H

#include <co/defines.h>

#if defined(COLLAGE_STATIC)
#  define CO_API
#elif defined(COLLAGE_SHARED)
#  define CO_API LB_DLLEXPORT
#else
#  define CO_API LB_DLLIMPORT
#endif

#endif //CO_API_H
