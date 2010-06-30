
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUNDEXITVISITOR_H
#define EQSERVER_COMPOUNDEXITVISITOR_H

#include "compoundVisitor.h" // base class

#include "channel.h"  // used in inline method
#include "compound.h" // used in inline method

namespace eq
{
namespace server
{
    class Channel;
    
    /**
     * The compound visitor exitializing a compound tree.
     */
    class CompoundExitVisitor : public CompoundVisitor
    {
    public:
        virtual ~CompoundExitVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound )
            {
                Channel* channel = compound->getChannel();
                if( compound->isDestination() && !channel->getSegment( ))
                {
                    EQASSERT( !channel->getView( ));
        
                    // old-school (non-Layout) destination channel, deactivate
                    //  compound layout destination channel compounds are
                    //  deactivated by canvas
                    compound->deactivate();
                }

                compound->restore();
                return TRAVERSE_CONTINUE;    
            }
    };
}
}
#endif // EQSERVER_COMPOUNDEXITVISITOR_H
