
/* Copyright (c) 2010, Daniel Pfeifer <daniel@pfeifer-mail.de> 
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
 * @file base/precompile.h
 *
 * Include headers that don't change... STL, Boost, other third-party stuff.
 * Don't include any Equalizer headers here!
 * Don't include this file anywhere!
 */

// This file references only auto generated files (OK to include here).
#include <co/base/defines.h>

#ifdef WIN32
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

#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef _MSC_VER
#  include <stdint.h>
#  include <sys/param.h>  // for MIN/MAX
#endif
