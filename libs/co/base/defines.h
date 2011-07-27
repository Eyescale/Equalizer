 
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef COBASE_DEFINES_H
#define COBASE_DEFINES_H

#ifdef __APPLE__
#  include <co/base/definesDarwin.h>
#endif
#ifdef __linux
#  include <co/base/definesLinux.h>
#endif
#ifdef _WIN32 //_MSC_VER
#  include <co/base/definesWin32.h>
#endif

// Defining our own min/max macros seems to be the only sane way to get this
// functionality across platforms thanks to some screwup in the MS header files.
#define EQ_MAX(a,b) ((a)>(b)?(a):(b)) //!< returns the maximum of two values
#define EQ_MIN(a,b) ((a)<(b)?(a):(b)) //!< returns the minimum of two values

/** A 'NULL' value for an uint32.*/
#define EQ_UNDEFINED_UINT32   (0xffffffffu)
/** The biggest usable value when using special uint32 values.*/
#define EQ_MAX_UINT32         (0xfffffff0u)

/** Constant defining 'wait forever' in methods with wait parameters. */
#define EQ_TIMEOUT_INDEFINITE 0xffffffffu // Attn: identical to Win32 INFINITE!
/** Constant defining use global default in methods with wait parameters. */
#define EQ_TIMEOUT_DEFAULT 0xfffffffeu

#ifdef __GNUC__
#  define CO_LIKELY(x)       __builtin_expect( (x), 1 )
#  define CO_UNLIKELY(x)     __builtin_expect( (x), 0 )
#else
#  define CO_LIKELY(x)       x
#  define CO_UNLIKELY(x)     x
#endif

#endif // COBASE_DEFINES_H
