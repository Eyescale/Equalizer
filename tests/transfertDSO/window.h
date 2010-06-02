
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

#ifndef EQ_PIXELBENCH_WINDOW_H
#define EQ_PIXELBENCH_WINDOW_H

#include <eq/eq.h>

namespace eqTransfertDSO
{
    class Window : public eq::Window
    {
    public:
        Window( eq::Pipe* parent ) : eq::Window( parent ) {}

    protected:
        virtual bool configInit( const uint32_t initID );
    };
}

#endif // EQ_PIXELBENCH_WINDOW_H
