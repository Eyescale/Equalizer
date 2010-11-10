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

#ifndef EQFABRIC_BASE_H
#define EQFABRIC_BASE_H

#include <eq/base/base.h>

#if defined(EQ_FABRIC_STATIC)
#  define EQ_FABRIC_DECL
#elif defined(EQ_FABRIC_SHARED)
#  define EQ_FABRIC_DECL EQ_DLLEXPORT
#else
#  define EQ_FABRIC_DECL EQ_DLLIMPORT
#endif

#endif //EQFABRIC_BASE_H
