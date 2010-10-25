
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

#ifndef EQ_ERROR_H
#define EQ_ERROR_H

#include <eq/base/base.h>
#include <eq/base/types.h> // EQ_KB definitions

namespace eq
{
    /** Defines errors produced by Equalizer classes. */
    enum Error
    {
        ERROR_FRAMEBUFFER_UNSUPPORTED = EQ_64KB, // use fabric::ERROR_CUSTOM
        ERROR_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
        ERROR_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
        ERROR_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
        ERROR_FRAMEBUFFER_INCOMPLETE_FORMATS,
        ERROR_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
        ERROR_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
        ERROR_FRAMEBUFFER_FULL_COLOR_TEXTURES,
        ERROR_FRAMEBUFFER_INITIALIZED,

        ERROR_CUDACONTEXT_DEVICE_NOTFOUND,
        ERROR_CUDACONTEXT_INIT_FAILED,
        ERROR_CUDACONTEXT_MISSING_SUPPORT,

        ERROR_CUSTOM = EQ_128KB
    };

    /** Print the error in a human-readable format. @version 1.0 */
    EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Error& error);
}
#endif // EQ_ERROR_H
