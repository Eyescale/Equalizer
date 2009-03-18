
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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

    virtual VisitorResult visitPre( Config* config )
        {
            const uint32_t  oldVersion = config->_headMatrix.getVersion();
            const VisitorResult result = _sync( &config->_headMatrix );
            const uint32_t  newVersion = config->_headMatrix.getVersion();

            if( oldVersion != newVersion )
                config->_updateEyes();

            return result;
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
