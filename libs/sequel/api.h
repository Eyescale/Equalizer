 
/* Copyright (c) 2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

/**
 * @file sequel/api.h
 *
 * Defines sequel API export macros.
 */
#ifndef SEQUEL_API_H
#define SEQUEL_API_H

#include <co/base/api.h>

#if defined(SEQUEL_STATIC)
#  define SEQ_API
#elif defined(SEQUEL_SHARED)
#  define SEQ_API EQ_DLLEXPORT
#else
#  define SEQ_API EQ_DLLIMPORT
#endif

#endif //SEQUEL_API_H
