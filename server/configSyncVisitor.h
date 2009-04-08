
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

    virtual VisitorResult visit( Observer* observer )
        {
            return _sync( observer );
        }
    virtual VisitorResult visitPre( Canvas* canvas )
        {
            return _sync( canvas );
        }
    virtual VisitorResult visit( View* view )
        { 
            return _sync( view );
        }
 
    private:
        const uint32_t            _nChanges;
        const net::ObjectVersion* _changes;
        uint32_t                  _current;

        VisitorResult _sync( net::Object* object )
            {
                EQASSERT( _current < _nChanges );
                const net::ObjectVersion& change = _changes[ _current ];

                if( change.id != object->getID( ))
                    return TRAVERSE_CONTINUE;

                object->sync( change.version );
                ++_current;

                if( _current == _nChanges )
                    return TRAVERSE_TERMINATE; // all done
                return TRAVERSE_CONTINUE;
            }
    };
}
}
#endif // EQSERVER_CONSTCONFIGVISITOR_H
