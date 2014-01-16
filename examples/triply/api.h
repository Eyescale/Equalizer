
/* Copyright (c) 2013, Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
 *               2013, Stefan.Eilemann@epfl.ch
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

#ifndef PLYLIB_API_H
#define PLYLIB_API_H

#if defined(_MSC_VER) || defined(__declspec)
#  define PLYLIB_DLLEXPORT __declspec(dllexport)
#  define PLYLIB_DLLIMPORT __declspec(dllimport)
#else // _MSC_VER
#  define PLYLIB_DLLEXPORT
#  define PLYLIB_DLLIMPORT
#endif // _MSC_VER

#if defined(PLYLIB_STATIC)
#  define PLYLIB_API
#elif defined(PLYLIB_SHARED)
#  define PLYLIB_API PLYLIB_DLLEXPORT
#else
#  define PLYLIB_API PLYLIB_DLLIMPORT
#endif

#endif /* PLYLIB_API_H */
