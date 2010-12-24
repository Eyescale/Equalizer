 
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQ_DEFINES_H
#define EQ_DEFINES_H

#include <co/base/defines.h>

#ifdef __APPLE__
#  include <eq/definesDarwin.h>
#endif
#ifdef __linux
#  include <eq/definesLinux.h>
#endif
#ifdef _WIN32 //_MSC_VER
#  include <eq/definesWin32.h>
#endif

#endif // EQ_DEFINES_H
