 
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * @file base/base.h
 *
 * Includes key system header files and defines essential base macros.
 */
#ifndef EQBASE_BASE_H
#define EQBASE_BASE_H

#include <eq/base/defines.h>

#ifdef _WIN32
#  ifndef _MSC_VER
#    define USE_SYS_TYPES_FD_SET
#  endif
#  define _USE_MATH_DEFINES
#  ifndef _WIN32_WINNT
#    ifdef EQ_USE_MAGELLAN
#      define _WIN32_WINNT 0x501 // XP
#    else
#      define _WIN32_WINNT 0x500 // 2000
#    endif
#  endif
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#  include <windows.h>
#  include <windef.h>
#endif

#if defined(_MSC_VER) || defined(__declspec)
#  define EQ_DLLEXPORT __declspec(dllexport)
#  define EQ_DLLIMPORT __declspec(dllimport)
#else // _MSC_VER
#  define EQ_DLLEXPORT
#  define EQ_DLLIMPORT
#endif // _MSC_VER

#if defined(EQ_BASE_STATIC)
#  define EQBASE_API
#elif defined(EQ_BASE_SHARED)
#  define EQBASE_API EQ_DLLEXPORT
#else
#  define EQBASE_API EQ_DLLIMPORT
#endif

#ifdef EQ_EXPORTS
#  define EQFABRIC_API EQ_DLLEXPORT
#endif

// Need to predefine server library exports for forward declaration of 
// eqsStartLocalServer
#ifdef EQSERVER_EXPORTS
#  define EQSERVER_EXPORT EQ_DLLEXPORT
#  define EQFABRIC_API EQSERVER_EXPORT
#else
#  define EQSERVER_EXPORT EQ_DLLIMPORT
#endif
#ifdef EQUALIZERADMIN_SHARED
#  define EQFABRIC_API EQ_DLLEXPORT
#endif
#ifndef EQFABRIC_API
#  define EQFABRIC_API EQ_DLLIMPORT
#endif

// Defining our own min/max macros seems to be the only sane way to get this
// functionality across platforms thanks to some screwup in the MS header files.
#define EQ_MAX(a,b) ((a)>(b)?(a):(b)) //!< returns the maximum of two values
#define EQ_MIN(a,b) ((a)<(b)?(a):(b)) //!< returns the minimum of two values

#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef _MSC_VER
#  include <stdint.h>
#  include <sys/param.h>  // for MIN/MAX
#endif

#ifdef Darwin
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#elif !defined(_WIN32)
extern "C" char **environ;
#endif

#include <eq/base/types.h>
#include <eq/base/compiler.h>

// defines
/** A 'NULL' value for an uint32, typically used for identifiers and versions.*/
#define EQ_UNDEFINED_UINT32   (0xffffffffu)
//#define EQ_UNDEFINED_FLOAT    (std::numeric_limits<float>::quiet_NaN( ))
//#define EQ_UNDEFINED_INT32    (0x7fffffffu)

/** Constant defining 'wait forever' in methods with wait parameters. */
#define EQ_TIMEOUT_INDEFINITE 0

#define VMMLIB_CUSTOM_CONFIG
#ifndef NDEBUG
#  define VMMLIB_SAFE_ACCESSORS
#endif
#define VMMLIB_ALIGN( var ) var

#endif //EQBASE_BASE_H
