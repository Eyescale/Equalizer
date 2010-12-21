
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUND_LISTENER_H
#define EQSERVER_COMPOUND_LISTENER_H

#include <co/base/os.h>

namespace eq
{
namespace server
{
    class Compound;

    /** A listener on various compound operations. */
    class CompoundListener
    {
    public:
        virtual ~CompoundListener(){}

        /** 
         * Notify that the compound tree below and including compound is about
         * to be updated.
         *
         * Called on each compound of the tree during update.
         *
         * @param compound the root compound of the tree to be updated.
         * @param frameNumber the new frame number.
         */
        virtual void notifyUpdatePre( Compound* compound, 
                                      const uint32_t frameNumber ) {}

        /** 
         * Notify that the compound has a new child.
         * 
         * @param compound the parent compound.
         * @param child the child compound.
         */
        virtual void notifyChildAdded( Compound* compound, Compound* child ){}

        /** 
         * Notify that the compound is about to remove a child.
         * 
         * @param compound the parent compound.
         * @param child the child compound.
         */
        virtual void notifyChildRemove( Compound* compound, Compound* child ){}
    };
}
}
#endif // EQSERVER_COMPOUND_LISTENER_H
