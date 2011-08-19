
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

#ifndef EQSERVER_COMPOUNDEXITVISITOR_H
#define EQSERVER_COMPOUNDEXITVISITOR_H

#include "compoundVisitor.h" // base class

#include <eq/fabric/eye.h>

namespace eq
{
namespace server
{
    /** The compound visitor exiting a compound tree. */
    class CompoundExitVisitor : public CompoundVisitor
    {
    public:
        virtual ~CompoundExitVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound )
            {
                Channel* channel = compound->getChannel();
                if( !channel || // non-channel root compounds
                    // old-school (non-Layout) destination channel
                    ( compound->isDestination() && !channel->getSegment( )))
                {
                    EQASSERT( !channel || !channel->getView( ));
                    uint32_t eyes = compound->getEyes();
                    if( eyes == fabric::EYE_UNDEFINED )
                        eyes = fabric::EYES_ALL;

                    compound->deactivate( eyes );
                }

                return TRAVERSE_CONTINUE;    
            }
    };
}
}
#endif // EQSERVER_COMPOUNDEXITVISITOR_H
