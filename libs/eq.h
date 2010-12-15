
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

#ifndef EQ_H
#define EQ_H

#pragma warning(push)
#pragma warning(disable : 4244) //conversion from .. to ..,possible loss of data

#include <eq/client.h>

#include <eq/util.h>
#include <eq/fabric.h>
#include <eq/net.h>
#include <eq/base.h>

#include <vmmlib/vmmlib.hpp>

#ifdef EQ_USE_DEPRECATED
namespace eqBase = ::eq::base;
namespace eqNet  = ::eq::net;
#endif

/** \mainpage Equalizer API Documentation
    \htmlinclude "RelNotes.dox"
*/

#pragma warning(pop)
#endif // EQ_H
