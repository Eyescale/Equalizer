
/* Copyright (c) 2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_COMPOUNDUPDATEACTIVATEVISITOR_H
#define EQSERVER_COMPOUNDUPDATEACTIVATEVISITOR_H

#include "compoundVisitor.h" // base class
#include "types.h"

namespace
{
class CompoundUpdateActivateVisitor : public eq::server::CompoundVisitor
{
public:
    explicit CompoundUpdateActivateVisitor( const uint32_t frameNumber )
        : _frameNumber( frameNumber ), _taskID( 0 ) {}
    virtual ~CompoundUpdateActivateVisitor() {}

    eq::server::VisitorResult visit( eq::server::Compound* compound )
    {
        compound->setTaskID( ++_taskID );
        compound->updateInheritData( _frameNumber );
        return eq::server::TRAVERSE_CONTINUE;
    }

private:
    const uint32_t _frameNumber;
    uint32_t _taskID;
};
}
#endif
