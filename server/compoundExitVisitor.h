
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

#include "compound.h" // used in inline method
#include "config.h"   // used in inline method
#include "frame.h"    // used in inline method

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
                Config* config = compound->getConfig();
                EQASSERT( config );

                const Frames& outputFrames = compound->getOutputFrames();
                for( Frames::const_iterator i = outputFrames.begin(); 
                     i != outputFrames.end(); ++i )
                {
                    Frame* frame = *i;
                    frame->flush();
                    config->deregisterObject( frame );
                }

                const Frames& inputFrames = compound->getInputFrames();
                for( Frames::const_iterator i = inputFrames.begin(); 
                     i != inputFrames.end(); ++i )
                {
                    Frame* frame = *i;
                    config->deregisterObject( frame );
                }
                return TRAVERSE_CONTINUE;    
            }
    };
}
}
#endif // EQSERVER_COMPOUNDEXITVISITOR_H
