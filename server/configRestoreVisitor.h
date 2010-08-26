
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
 * Copyright (c) 2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQSERVER_CONFIGRESTOREVISITOR_H
#define EQSERVER_CONFIGRESTOREVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
namespace
{
    /** Restore all data which may have been modified by an application. */
    class ConfigRestoreVisitor : public ConfigVisitor
    {
    public:
        ConfigRestoreVisitor() {}
        virtual ~ConfigRestoreVisitor() {}
        virtual VisitorResult visitPre( Compound* compound )
            {
                compound->restore();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visitPre( Config* config )
            {
                config->restore();
                config->commit();
                return TRAVERSE_CONTINUE;
            }
        virtual VisitorResult visit( Observer* observer )
            {
                return _restore( observer );
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                return _restore( canvas );
            }
        virtual VisitorResult visit( Segment* segment )
            {
                return _restore( segment );
            }
        virtual VisitorResult visitPre( Layout* layout )
            {
                return _restore( layout );
            }
        virtual VisitorResult visit( View* view )
            {
                return _restore( view );
            }
        virtual VisitorResult visitPre( Node* node )
            {
                return _restore( node );
            }
        virtual VisitorResult visitPre( Pipe* pipe )
            {
                return _restore( pipe );
            }
        virtual VisitorResult visitPre( Window* window )
            {
                return _restore( window );
            }
        virtual VisitorResult visit( Channel* channel )
            {
                return _restore( channel );
            }

    private:
        VisitorResult _restore( fabric::Object* object )
            {
                object->restore();
                object->commit();
                return TRAVERSE_CONTINUE;
            }
    };
}
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
