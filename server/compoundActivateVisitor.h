
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUNDACTIVATEVISITOR_H
#define EQSERVER_COMPOUNDACTIVATEVISITOR_H

#include "compoundVisitor.h" // base class

#include "compound.h" // used in inline method

namespace eq
{
namespace server
{
    /** The compound visitor (de-)activating a compound tree. */
    class CompoundActivateVisitor : public CompoundVisitor
    {
    public:
        CompoundActivateVisitor( const bool activate ) : _activate( activate ){}
        virtual ~CompoundActivateVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound )
            {
                compound->setActive( _activate );
                
                Channel* channel = compound->getChannel();
                if( channel )
                {
                    if( _activate )
                        channel->activate();
                    else
                        channel->deactivate();
                }
                return TRAVERSE_CONTINUE;
            }

    private:
            const bool _activate;
    };
}
}
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
