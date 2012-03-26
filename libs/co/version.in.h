
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef CO_VERSION_H
#define CO_VERSION_H

#include <co/api.h>
#include <co/base/types.h>
#include <string>

namespace co
{
    // Collage version macros and functions
    /** The current major version. @version 0.4 */
#   define CO_VERSION_MAJOR @CO_VERSION_MAJOR@

    /** The current minor version. @version 0.4 */
#   define CO_VERSION_MINOR @CO_VERSION_MINOR@

    /** The current patch level. @version 0.4 */
#   define CO_VERSION_PATCH @CO_VERSION_PATCH@

    /** The git wc hash revision, may be 0. @version 0.4 */
#   define CO_VERSION_REVISION @EQ_REVISION@

    /** The current DSO binary revision. @version 0.4 */
#   define CO_VERSION_ABI @VERSION_ABI@

/** True if the current version is newer than the given one. @version 0.4 */
#   define CO_VERSION_GT( MAJOR, MINOR, PATCH )                         \
    ( (CO_VERSION_MAJOR>MAJOR) ||                                       \
      (CO_VERSION_MAJOR==MAJOR && (CO_VERSION_MINOR>MINOR ||            \
                          (CO_VERSION_MINOR==MINOR && CO_VERSION_PATCH>PATCH))))

/** True if the current version is equal or newer to the given. @version 0.4 */
#   define CO_VERSION_GE( MAJOR, MINOR, PATCH )                         \
    ( (CO_VERSION_MAJOR>MAJOR) ||                                       \
      (CO_VERSION_MAJOR==MAJOR && (CO_VERSION_MINOR>MINOR ||            \
                         (CO_VERSION_MINOR==MINOR && CO_VERSION_PATCH>=PATCH))))

/** True if the current version is older than the given one. @version 0.4 */
#   define CO_VERSION_LT( MAJOR, MINOR, PATCH )                         \
    ( (CO_VERSION_MAJOR<MAJOR) ||                                       \
      (CO_VERSION_MAJOR==MAJOR && (CO_VERSION_MINOR<MINOR ||            \
                          (CO_VERSION_MINOR==MINOR && CO_VERSION_PATCH<PATCH))))

/** True if the current version is older or equal to the given. @version 0.4 */
#   define CO_VERSION_LE( MAJOR, MINOR, PATCH )                         \
    ( (CO_VERSION_MAJOR<MAJOR) ||                                       \
      (CO_VERSION_MAJOR==MAJOR && (CO_VERSION_MINOR<MINOR ||            \
                         (CO_VERSION_MINOR==MINOR && CO_VERSION_PATCH<=PATCH))))

    /** Information about the current Collage version. */
    class CO_API Version
    {
    public:
        /** @return the current major version of Collage. @version 0.4 */
        static uint32_t getMajor();

        /** @return the current minor version of Collage. @version 0.4 */
        static uint32_t getMinor();

        /** @return the current patch level of Collage. @version 0.4 */
        static uint32_t getPatch();

        /** @return the current revision of Collage. @version 0.4 */
        static std::string getRevision();

        /** @return the current DSO binary revision. @version 0.4 */
        static uint32_t getABI();

        /** @return the current Collage version (MMmmpp). @version 0.4 */
        static uint32_t getInt();

        /** @return the current Collage version (MM.mmpp). @version 0.4 */
        static float    getFloat();

        /**
         * @return the current Collage version (MM.mm.pp[.rr]).
         * @version 1.1
         */
        static std::string getString();

        /**
         * @return true if the link-time and compile-time DSO versions are
         *         compatible.
         */ 
        static bool check()
        { return getMajor()==CO_VERSION_MAJOR && getMinor()==CO_VERSION_MINOR; }
    };
}

#endif //CO_VERSION_H
