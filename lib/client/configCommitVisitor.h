
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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

        virtual VisitorResult visitPre( Config* config )
            { 
                _commit( &config->_headMatrix );
                return TRAVERSE_CONTINUE; 
            }
        virtual VisitorResult visitPre( Canvas* canvas )
            {
                if( canvas->getDirty() & Canvas::DIRTY_LAYOUT )
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
