
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSEQUEL_DETAIL_CHANNEL_H
#define EQSEQUEL_DETAIL_CHANNEL_H

#include <eq/sequel/types.h>

#include <eq/channel.h> // base class

namespace seq
{
namespace detail
{
    class Channel : public eq::Channel
    {
    public:
        Channel( eq::Window* parent );

        //Config* getConfig();

    protected:
        virtual ~Channel();

    private:
    };
}
}

#endif // EQSEQUEL_DETAIL_CHANNEL_H
