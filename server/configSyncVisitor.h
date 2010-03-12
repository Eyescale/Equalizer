
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

#ifndef EQSERVER_CONFIGSYNCVISITOR_H
#define EQSERVER_CONFIGSYNCVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
    /** Synchronizes changes from the app at the beginning of each frame. */
    class ConfigSyncVisitor : public ConfigVisitor
    {
    public:
        ConfigSyncVisitor( const uint32_t nChanges, 
                           const net::ObjectVersion* changes )
                : _nChanges( nChanges ), _changes( changes ), _current( 0 ) {}
        virtual ~ConfigSyncVisitor() {}

    virtual VisitorResult visitPre( Node* node ) { return TRAVERSE_PRUNE; }
    virtual VisitorResult visit( Observer* observer )
        {
            _sync( observer );
            return TRAVERSE_CONTINUE;
        }
    virtual VisitorResult visitPre( Canvas* canvas )
        {
            _sync( canvas );
            return TRAVERSE_CONTINUE;
        }
    virtual VisitorResult visit( View* view )
        {
            view->sync();
            return TRAVERSE_CONTINUE;
        }
    virtual VisitorResult visitPre( Compound* compound )
        {
            EQASSERT( _current == _nChanges );
            return TRAVERSE_TERMINATE;
        }
 
    private:
        const uint32_t            _nChanges;
        const net::ObjectVersion* _changes;
        uint32_t                  _current;

        void _sync( net::Object* object )
            {
                if( _current == _nChanges )
                    return;

                EQASSERT( _current < _nChanges );
                const net::ObjectVersion& change = _changes[ _current ];

                if( change.identifier != object->getID( ))
                    return;

                object->sync( change.version );
                ++_current;
            }
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
