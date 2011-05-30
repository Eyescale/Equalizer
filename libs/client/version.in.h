
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

#include <eq/api.h>
#include <co/base/types.h>
#include <string>

namespace eq
{
    // Equalizer version macros and functions
    /** The current major version. @version 1.0 */
#   define EQ_VERSION_MAJOR ${VERSION_MAJOR}

    /** The current minor version. @version 1.0 */
#   define EQ_VERSION_MINOR ${VERSION_MINOR}

    /** The current patch level. @version 1.0 */
#   define EQ_VERSION_PATCH ${VERSION_SUB}

    /** The git wc hash revision, may be 0. @version 1.0 */
#   define EQ_VERSION_REVISION ${EQ_REVISION}

    /** The current DSO binary revision. @version 1.0 */
#   define EQ_VERSION_ABI ${VERSION_ABI}

/** True if the current version is newer than the given one. @version 1.0 */
#   define EQ_VERSION_GT( MAJOR, MINOR, PATCH )                         \
    ( (EQ_VERSION_MAJOR>MAJOR) ||                                       \
      (EQ_VERSION_MAJOR==MAJOR && (EQ_VERSION_MINOR>MINOR ||            \
                          (EQ_VERSION_MINOR==MINOR && EQ_VERSION_PATCH>PATCH))))

/** True if the current version is equal or newer to the given. @version 1.0 */
#   define EQ_VERSION_GE( MAJOR, MINOR, PATCH )                         \
    ( (EQ_VERSION_MAJOR>MAJOR) ||                                       \
      (EQ_VERSION_MAJOR==MAJOR && (EQ_VERSION_MINOR>MINOR ||            \
                         (EQ_VERSION_MINOR==MINOR && EQ_VERSION_PATCH>=PATCH))))

/** True if the current version is older than the given one. @version 1.0 */
#   define EQ_VERSION_LT( MAJOR, MINOR, PATCH )                         \
    ( (EQ_VERSION_MAJOR<MAJOR) ||                                       \
      (EQ_VERSION_MAJOR==MAJOR && (EQ_VERSION_MINOR<MINOR ||            \
                          (EQ_VERSION_MINOR==MINOR && EQ_VERSION_PATCH<PATCH))))

/** True if the current version is older or equal to the given. @version 1.0 */
#   define EQ_VERSION_LE( MAJOR, MINOR, PATCH )                         \
    ( (EQ_VERSION_MAJOR<MAJOR) ||                                       \
      (EQ_VERSION_MAJOR==MAJOR && (EQ_VERSION_MINOR<MINOR ||            \
                         (EQ_VERSION_MINOR==MINOR && EQ_VERSION_PATCH<=PATCH))))

    /** Information about the current Equalizer version. */
    class EQ_API Version
    {
    public:
        /** @return the current major version of Equalizer. @version 1.0 */
        static uint32_t getMajor();

        /** @return the current minor version of Equalizer. @version 1.0 */
        static uint32_t getMinor();

        /** @return the current patch level of Equalizer. @version 1.0 */
        static uint32_t getPatch();

        /** @return the current revision of Equalizer. @version 1.0 */
        static std::string getRevision();

        /** @return the current DSO binary revision. @version 1.1.1 */
        static uint32_t getABI();

        /** @return the current Equalizer version (MMmmpp). @version 1.0 */
        static uint32_t getInt();

        /** @return the current Equalizer version (MM.mmpp). @version 1.0 */
        static float    getFloat();

        /**
         * @return the current Equalizer version (MM.mm.pp[.rr]).
         * @version 1.0
         */
        static std::string getString();
    };
}

#endif //EQ_VERSION_H
