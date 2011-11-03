
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

#ifndef GPUSD_REMOTE_API_H
#define GPUSD_REMOTE_API_H

#if defined(_MSC_VER) || defined(__declspec)
#  define GPUSD_REMOTE_DLLEXPORT __declspec(dllexport)
#  define GPUSD_REMOTE_DLLIMPORT __declspec(dllimport)
#else
#  define GPUSD_REMOTE_DLLEXPORT
#  define GPUSD_REMOTE_DLLIMPORT
#endif

#if defined(GPUSD_REMOTE_STATIC)
#  define GPUSD_REMOTE_API
#elif defined(GPUSD_REMOTE_SHARED)
#  define GPUSD_REMOTE_API GPUSD_REMOTE_DLLEXPORT
#else
#  define GPUSD_REMOTE_API GPUSD_REMOTE_DLLIMPORT
#endif

#endif //GPUSD_REMOTE_API_H
