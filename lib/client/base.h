 
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

#ifndef EQCLIENT_BASE_H
#define EQCLIENT_BASE_H

#include <eq/base/base.h>

#if defined(EQ_CLIENT_STATIC)
#  define EQ_CLIENT_DECL
#elif defined(EQ_CLIENT_SHARED)
#  define EQ_CLIENT_DECL EQ_DLLEXPORT
#else
#  define EQ_CLIENT_DECL EQ_DLLIMPORT
#endif

#endif /* EQCLIENT_BASE_H */
