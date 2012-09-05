
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 *               2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_COMMANDTTYPE_H
#define EQFABRIC_COMMANDTTYPE_H

#include <co/commands.h> // 'base' enum

namespace eq
{
namespace fabric
{
    /** Command types to identify the target of a command. */
    enum CommandType
    {
        COMMANDTYPE_EQ_CLIENT = co::COMMANDTYPE_CO_CUSTOM, // 128
        COMMANDTYPE_EQ_SERVER,
        COMMANDTYPE_EQ_CUSTOM = 1<<8 // 256
    };
}
}
#endif // EQFABRIC_COMMANDTTYPE_H
