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

#ifndef CO_BASE_H
#define CO_BASE_H

#include <eq/base/base.h>

namespace co{}
/* deprecated namespace */
namespace eq{ namespace net = co; }

#if defined(EQ_NET_STATIC)
#  define CO_API
#elif defined(EQ_NET_SHARED)
#  define CO_API EQ_DLLEXPORT
#else
#  define CO_API EQ_DLLIMPORT
#endif

#endif //CO_BASE_H
