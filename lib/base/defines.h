 
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

#ifndef EQBASE_DEFINES_H
#define EQBASE_DEFINES_H

#ifdef __APPLE__
#  ifdef Darwin
#    include <eq/base/definesDarwin.h>
#  else // must be XCode build
#    include <eq/base/definesXCode.h>
#  endif
#endif
#ifdef __linux
#  include <eq/base/definesLinux.h>
#endif
#ifdef _WIN32 //_MSC_VER
#  include <eq/base/definesWin32.h>
#endif

#endif // EQBASE_DEFINES_H
