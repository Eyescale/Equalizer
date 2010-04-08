
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

#ifndef EQFABRIC_GLOBAL_H
#define EQFABRIC_GLOBAL_H

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace fabric
{
    /** Global parameter handling for the Equalizer fabric namespace. */
    class Global
    {
    public:
        /** 
         * Set the default Equalizer server.
         * 
         * @param server the default server.
         */
        EQ_EXPORT static void setServer( const std::string& server );

        /** @return the default Equalizer server. */
        EQ_EXPORT static const std::string& getServer();
    };
}
}
#endif // EQFABRIC_GLOBAL_H

