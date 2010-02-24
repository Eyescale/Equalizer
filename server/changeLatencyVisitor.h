
/* Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com> 
 *               2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQSERVER_CHANGELATENCYVISITOR_H
#define EQSERVER_CHANGELATENCYVISITOR_H

#include "compound.h"
#include "configVisitor.h"
#include "frame.h"
#include "node.h"
#include "view.h"

namespace eq
{
namespace server
{
/**
 * The Change Latency visitor modifies the latency on all relevant objects.
 */
class ChangeLatencyVisitor : public ConfigVisitor
{
public:
    ChangeLatencyVisitor( const uint32_t latency ) : _latency( latency ) {}

    virtual VisitorResult visit( Compound* compound )
    { 
        const std::vector< Frame* >& outputFrames = compound->getOutputFrames();
        for( std::vector<Frame*>::const_iterator i = outputFrames.begin(); 
             i != outputFrames.end(); ++i )
        {
            Frame* frame = *i;
            frame->setAutoObsolete( _latency );
        }

        const std::vector< Frame* >& inputFrames = compound->getInputFrames();
        for( std::vector<Frame*>::const_iterator i = inputFrames.begin(); 
             i != inputFrames.end(); ++i )
        {
            Frame* frame = *i;
            frame->setAutoObsolete( _latency );
        }
        return TRAVERSE_CONTINUE; 
    }

    virtual VisitorResult visit( Node* node )
    {
        // change latency in barrier
        node->changeLatency( _latency );
        return TRAVERSE_CONTINUE; 
    }

    virtual VisitorResult visitLeaf( View* view )
    {
        // change latency in barrier
        view->setAutoObsolete( _latency );
        return TRAVERSE_CONTINUE; 
    }

private:
    const uint32_t _latency;
};
}
}
#endif // EQSERVER_CHANGELATENCYVISITOR
