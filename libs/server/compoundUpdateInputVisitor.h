
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

#ifndef EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H
#define EQSERVER_COMPOUNDUPDATEINPUTVISITOR_H

#include "compoundVisitor.h" // base class

#include <co/base/hash.h>

namespace eq
{
namespace server
{
    class Channel;
    class Frame;

    /**
     * The compound visitor updating the inherit input of a compound tree.
     */
    class CompoundUpdateInputVisitor : public CompoundVisitor
    {
    public:
        CompoundUpdateInputVisitor( 
            const stde::hash_map<std::string, Frame*>& outputFrames );
        virtual ~CompoundUpdateInputVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound );

    private:
        const stde::hash_map<std::string, Frame*>& _outputFrames;

        void _updateZoom( const Compound* compound, Frame* frame, 
                          const Frame* outputFrame );
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
