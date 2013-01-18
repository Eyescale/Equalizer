
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H
#define EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H

#include "compoundVisitor.h" // base class
#include "compound.h"        // nested type

namespace eq
{
namespace server
{
    /** The compound visitor updating the inherit input of a compound tree. */
    class CompoundUpdateInputVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateInputVisitor( const Compound::FrameMap& outputFrames,
                                   const Compound::TileQueueMap& outputQueues );
        virtual ~CompoundUpdateInputVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
        const Compound::FrameMap& _outputFrames;
        const Compound::TileQueueMap& _outputQueues;

        void _updateQueues( const Compound* compound );
        void _updateFrames( Compound* compound );
        void _updateZoom( const Compound* compound, Frame* frame, 
                          const Frame* outputFrame );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
