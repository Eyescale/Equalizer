
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "window.h"

namespace eqTransfertDSO
{

bool Window::configInit( const uint32_t initID )
{
    setIAttribute( IATTR_PLANES_ALPHA,   8 ); // enforce visual with alpha
    setIAttribute( IATTR_PLANES_STENCIL, 8 ); // enforce visual with stencil

    return eq::Window::configInit( initID );
}
}

