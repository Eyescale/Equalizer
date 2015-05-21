
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

#ifndef EQSEQUEL_ERROR_H
#define EQSEQUEL_ERROR_H

#include <eq/error.h>

namespace seq
{
    /** Defines errors produced by Sequel. */
    enum Error
    {
        ERROR_SEQUEL_MAPOBJECT_FAILED = eq::ERROR_CUSTOM,
        ERROR_SEQUEL_CREATERENDERER_FAILED
    };

    /** Set up Sequel-specific error codes. */
    void initErrors();

    /** Clear Sequel-specific error codes. */
    void exitErrors();
}

#endif // EQSEQUEL_ERROR_H
