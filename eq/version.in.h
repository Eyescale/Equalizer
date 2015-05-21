
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/client/api.h>
#include <lunchbox/types.h>
#include <string>

namespace eq
{
    // Equalizer version macros and functions
    /** The current major version. @version 1.0 */
#   define EQ_VERSION_MAJOR @VERSION_MAJOR@

    /** The current minor version. @version 1.0 */
#   define EQ_VERSION_MINOR @VERSION_MINOR@

    /** The current patch level. @version 1.0 */
#   define EQ_VERSION_PATCH @VERSION_PATCH@

    /** The git wc hash revision, may be 0. @version 1.0 */
#   define EQ_VERSION_REVISION @GIT_REVISION@

    /** The current DSO binary revision. @version 1.0 */
#   define EQ_VERSION_ABI @VERSION_ABI@

    /** The Collage version used for linking. @version 1.5.1 */
#   define EQ_COLLAGE_VERSION @COLLAGE_VERSION@

    /** The Lunchbox version used for linking. @version 1.5.1 */
#   define EQ_LUNCHBOX_VERSION @LUNCHBOX_VERSION@

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

        /**
         * @return true if the link-time and compile-time DSO versions are
         *         compatible.
         */ 
        static bool check()
        { return getMajor()==EQ_VERSION_MAJOR && getMinor()==EQ_VERSION_MINOR; }
    };
}

#endif //EQ_VERSION_H
