
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

#ifndef GPUSD_VERSION_H
#define GPUSD_VERSION_H

#include <string>

namespace gpusd
{
/** The current major version. */
#   define GPUSD_VERSION_MAJOR @VERSION_MAJOR@

/** The current minor version. */
#   define GPUSD_VERSION_MINOR @VERSION_MINOR@

/** The current patch level. */
#   define GPUSD_VERSION_PATCH @VERSION_PATCH@

/** True if the current version is newer than the given one. */
#   define GPUSD_VERSION_GT( MAJOR, MINOR, PATCH )                      \
    ( (GPUSD_VERSION_MAJOR>MAJOR) ||                                    \
      (GPUSD_VERSION_MAJOR==MAJOR && (GPUSD_VERSION_MINOR>MINOR ||      \
                                      (GPUSD_VERSION_MINOR==MINOR &&    \
                                       GPUSD_VERSION_PATCH>PATCH))))

/** True if the current version is equal or newer to the given. */
#   define GPUSD_VERSION_GE( MAJOR, MINOR, PATCH )                      \
    ( (GPUSD_VERSION_MAJOR>MAJOR) ||                                    \
      (GPUSD_VERSION_MAJOR==MAJOR && (GPUSD_VERSION_MINOR>MINOR ||      \
                                      (GPUSD_VERSION_MINOR==MINOR &&    \
                                       GPUSD_VERSION_PATCH>=PATCH))))

/** True if the current version is older than the given one. */
#   define GPUSD_VERSION_LT( MAJOR, MINOR, PATCH )                      \
    ( (GPUSD_VERSION_MAJOR<MAJOR) ||                                    \
      (GPUSD_VERSION_MAJOR==MAJOR && (GPUSD_VERSION_MINOR<MINOR ||      \
                                      (GPUSD_VERSION_MINOR==MINOR &&    \
                                       GPUSD_VERSION_PATCH<PATCH))))

/** True if the current version is older or equal to the given. */
#   define GPUSD_VERSION_LE( MAJOR, MINOR, PATCH )                      \
    ( (GPUSD_VERSION_MAJOR<MAJOR) ||                                    \
      (GPUSD_VERSION_MAJOR==MAJOR && (GPUSD_VERSION_MINOR<MINOR ||      \
                                      (GPUSD_VERSION_MINOR==MINOR &&    \
                                       GPUSD_VERSION_PATCH<=PATCH))))

/** Information about the current GPU-SD version. */
class Version
{
public:
    /** @return the current major version of Gpusd. */
    static int getMajor();

    /** @return the current minor version of Gpusd. */
    static int getMinor();

    /** @return the current patch level of Gpusd. */
    static int getPatch();

    /** @return the current Gpusd version (MM.mm.pp). */
    static std::string getString();
};

}

#endif // GPUSD_VERSION_H
