
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CONFIGCOMMITVISITOR_H
#define EQ_CONFIGCOMMITVISITOR_H

#include <eq/client/configVisitor.h> // base class

namespace eq
{
    class Config;

    /** Commits all dirty config object at the beginning of each frame. */
    class ConfigCommitVisitor : public ConfigVisitor
    {
    public:
        ConfigCommitVisitor() : _needFinish( false ) {}
        virtual ~ConfigCommitVisitor() {}

        virtual VisitorResult visit( Observer* observer )
            { 
                _commit( observer );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                if( canvas->hasDirtyLayout( ))
                    _needFinish = true;

                _commit( canvas );
                return TRAVERSE_PRUNE; // no need to visit segments
            }
        virtual VisitorResult visit( View* view )
            { 
                _commit( view );
                return TRAVERSE_CONTINUE; 
            }
        
        bool needsFinish() const { return _needFinish; }

        const std::vector< net::ObjectVersion >& getChanges() const
            { return _changes; }

    private:
        bool _needFinish;
        std::vector< net::ObjectVersion > _changes;
        
        void _commit( net::Object* object )
            {
                const uint32_t oldVersion = object->getVersion();
                const uint32_t newVersion = object->commit();

                if( oldVersion != newVersion )
                    _changes.push_back( net::ObjectVersion( object->getID(),
                                                            newVersion ));
            }
    };
}

#endif // EQ_CONFIGDESERIALIZER_H
