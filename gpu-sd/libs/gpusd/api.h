
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef GPUSD_API_H
#define GPUSD_API_H

#if defined(_MSC_VER) || defined(__declspec)
#  define GPUSD_DLLEXPORT __declspec(dllexport)
#  define GPUSD_DLLIMPORT __declspec(dllimport)
#else
#  define GPUSD_DLLEXPORT
#  define GPUSD_DLLIMPORT
#endif

#if defined(GPUSD_STATIC)
#  define GPUSD_API
#elif defined(GPUSD_SHARED)
#  define GPUSD_API GPUSD_DLLEXPORT
#else
#  define GPUSD_API GPUSD_DLLIMPORT
#endif

#endif //GPUSD_API_H
