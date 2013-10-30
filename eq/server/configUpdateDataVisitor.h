
/* Copyright (c) 2008-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQSERVER_CONFIGUPDATEDATAVISITOR_H
#define EQSERVER_CONFIGUPDATEDATAVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
/** The config visitor updating config data after the compound update */
class ConfigUpdateDataVisitor : public ConfigVisitor
{
public:
    ConfigUpdateDataVisitor();
    virtual ~ConfigUpdateDataVisitor() {}

    VisitorResult visitPre( Node* node ) override;
    VisitorResult visitPost( Node* node ) override;
    VisitorResult visitPre( Pipe* pipe ) override;
    VisitorResult visitPost( Pipe* pipe ) override;
    VisitorResult visitPre( Window* window ) override;
    VisitorResult visitPost( Window* window ) override;
    VisitorResult visit( Channel* channel ) override;

    // No need to traverse compounds
    VisitorResult visitPre( Compound* ) override { return TRAVERSE_PRUNE; }

private:
    const Channel* _lastDrawChannel;
    const Window*  _lastDrawWindow;
    const Pipe*    _lastDrawPipe;
};
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
