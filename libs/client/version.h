
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_VERSION_H
#define EQ_VERSION_H

#include <co/base/os.h>

#include <string>

namespace eq
{
    // Equalizer version macros and functions
#   define EQ_VERSION_MAJOR 0 //!< The current major version. @version 1.0
#   define EQ_VERSION_MINOR 9 //!< The current minor version. @version 1.0
#   define EQ_VERSION_PATCH 3 //!< The current patch level. @version 1.0

    /** Information about the current Equalizer version. */
    class Version
    {
    public:
        /** @return the current major version of Equalizer. @version 1.0 */
        static uint32_t getMajor();

        /** @return the current minor version of Equalizer. @version 1.0 */
        static uint32_t getMinor();

        /** @return the current patch level of Equalizer. @version 1.0 */
        static uint32_t getPatch();

        /** @return the current Equalizer version (MMmmpp). @version 1.0 */
        static uint32_t getInt();

        /** @return the current Equalizer version (MM.mmpp). @version 1.0 */
        static float    getFloat();

        /** @return the current Equalizer version (MM.mm.pp). @version 1.0 */
        static std::string getString();
    };
}

#endif //EQ_VERSION_H
