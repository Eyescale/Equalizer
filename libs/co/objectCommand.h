
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_OBJECTCOMMAND_H
#define CO_OBJECTCOMMAND_H

namespace co
{
    enum ObjectCommand
    {
        CMD_OBJECT_INSTANCE,
        CMD_OBJECT_DELTA,
        CMD_OBJECT_SLAVE_DELTA,
        CMD_OBJECT_COMMIT,
        CMD_OBJECT_PUSH,
        CMD_OBJECT_OBSOLETE,
        CMD_OBJECT_MAX_VERSION
        // check that not more then CMD_OBJECT_CUSTOM have been defined!
    };
}

#endif // CO_OBJECTCOMMAND_H

