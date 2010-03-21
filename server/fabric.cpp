
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "types.h"
#include "node.h"
#include "pipe.h"
#include "channel.h"
#include "window.h"
#include "../lib/fabric/window.cpp"
#include "../lib/fabric/pipe.cpp"
#include "../lib/fabric/channel.cpp"

template class eq::fabric::Channel< eq::server::Channel, eq::server::Window >;

template class eq::fabric::Window< eq::server::Pipe, 
                                   eq::server::Window, 
                                   eq::server::Channel >;

template class eq::fabric::Pipe< eq::server::Node, 
                                 eq::server::Pipe, 
                                 eq::server::Window >;

