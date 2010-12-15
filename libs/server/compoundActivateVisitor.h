
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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

namespace eq
{
namespace server
{
    /** The compound visitor (de-)activating channels of a compound tree. */
    class CompoundActivateVisitor : public CompoundVisitor
    {
    public:
        CompoundActivateVisitor( const bool activate, const eq::Eye eye ) 
            : _activate( activate )
            , _eye( eye ) {}
        virtual ~CompoundActivateVisitor() {}

        /** Visit all compounds. */
        virtual VisitorResult visit( Compound* compound )
            {
                Channel* channel = compound->getChannel();
                if( channel && compound->testInheritEye( _eye ))
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
        const eq::Eye _eye;
    };
}
}
#endif // EQSERVER_COMPOUNDACTIVATEVISITOR_H
