
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSEQUEL_OBJECTTYPE_H
#define EQSEQUEL_OBJECTTYPE_H

#include <co/objectFactory.h>

namespace seq
{
/** Built-in object types. @version 1.0 */
enum ObjectType
{
    /** The object passed to Application::init() */
    OBJECTTYPE_INITDATA = co::OBJECTTYPE_CUSTOM,
    OBJECTTYPE_FRAMEDATA,   //!< The object passed to Application::run()
    OBJECTTYPE_CUSTOM = 100 //!< Application-specific objects
};
}
#endif // EQSEQUEL_OBJECTTYPE_H
