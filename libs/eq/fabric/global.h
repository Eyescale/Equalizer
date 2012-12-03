
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>

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
        EQFABRIC_API static void setServer( const std::string& server );

        /** @return the default Equalizer server. */
        EQFABRIC_API static const std::string& getServer();

        /** @return the error registry. @version 1.5 */
        EQFABRIC_API static ErrorRegistry& getErrorRegistry();

        EQFABRIC_API static void setFlags( const uint32_t flags );//!< @internal
        EQFABRIC_API static uint32_t getFlags(); //!< @internal
    };
}
}
#endif // EQFABRIC_GLOBAL_H

