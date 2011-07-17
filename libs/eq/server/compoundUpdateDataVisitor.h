
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUNDUPDATEDATAVISITOR_H
#define EQSERVER_COMPOUNDUPDATEDATAVISITOR_H

#include "compoundVisitor.h" // base class
#include "types.h"

namespace eq
{
namespace server
{
    class Channel;
    
    /**
     * The compound visitor updating the inherit data of a compound tree.
     */
    class CompoundUpdateDataVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateDataVisitor( const uint32_t frameNumber );
        virtual ~CompoundUpdateDataVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
        const uint32_t _frameNumber;
        uint32_t _taskID;

        void _updateDrawFinish( Compound* compound );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
