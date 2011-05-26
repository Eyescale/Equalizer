 
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * @file base/api.h
 *
 * Defines export visibility macros for co::base.
 */
#ifndef COBASE_API_H
#define COBASE_API_H

#include <co/base/defines.h>

#if defined(_MSC_VER) || defined(__declspec)
#  define EQ_DLLEXPORT __declspec(dllexport)
#  define EQ_DLLIMPORT __declspec(dllimport)
#else // _MSC_VER
#  define EQ_DLLEXPORT
#  define EQ_DLLIMPORT
#endif // _MSC_VER

#if defined(CO_BASE_STATIC)
#  define COBASE_API
#elif defined(CO_BASE_SHARED)
#  define COBASE_API EQ_DLLEXPORT
#else
#  define COBASE_API EQ_DLLIMPORT
#endif

#endif //COBASE_API_H
