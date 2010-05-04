 
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
 * @file admin/base.h
 *
 * Includes key system header files and defines essential admin macros.
 */
#ifndef EQADMIN_BASE_H
#define EQADMIN_BASE_H

#include <eq/base/base.h>

#ifdef _MSC_VER
#  ifdef EQADMIN_EXPORTS
#    define EQADMIN_EXPORT EQ_DLLEXPORT
#  else
#    define EQADMIN_EXPORT EQ_DLLIMPORT
#  endif
#else // WIN32
/** Mark the following function as exported in the Equalizer DSO. @internal */
#  define EQADMIN_EXPORT
#endif

#endif //EQADMIN_BASE_H
