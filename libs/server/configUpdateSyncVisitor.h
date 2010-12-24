
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQSERVER_CONFIGUPDATESYNCVISITOR_H
#define EQSERVER_CONFIGUPDATESYNCVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
namespace
{

class ConfigUpdateSyncVisitor : public ConfigVisitor
{
public:
    ConfigUpdateSyncVisitor() : _result( true ), _sync( false ) {}
    virtual ~ConfigUpdateSyncVisitor() {}

    virtual VisitorResult visitPre( Config* config )
        {
            _result = true;
            _sync = false;
            _error = ERROR_NONE;
            return TRAVERSE_CONTINUE;
        }

    virtual VisitorResult visitPre( Node* node )
        { return _updateDown( node ); }
    virtual VisitorResult visitPost( Node* node )
        {
            const VisitorResult& result = _updateUp( node, "node" );
            node->flushSendBuffer();
            return result;
        }

    virtual VisitorResult visitPre( Pipe* pipe )
        { return _updateDown( pipe ); }
    virtual VisitorResult visitPost( Pipe* pipe )
        { return _updateUp( pipe, "pipe" ); }

    virtual VisitorResult visitPre( Window* window )
        { return _updateDown( window ); }
    virtual VisitorResult visitPost( Window* window )
        { return _updateUp( window, "window" ); }

    virtual VisitorResult visit( Channel* channel )
        {
            return _updateUp( channel, "channel" );
        }

    bool getResult() const { return _result; }
    bool needsSync() const { return _sync; }
    co::base::Error getError() const
        { EQASSERT( _error != ERROR_NONE ); return _error; }

private:
    bool _result; // success or failure
    bool _sync;   // call again after init failure
    co::base::Error _error; // error message

    template< class T > VisitorResult _updateDown( T* entity ) const
        {
            const uint32_t state = entity->getState() & ~STATE_DELETE;
            switch( state )
            {
                case STATE_INITIALIZING:
                case STATE_INIT_FAILED:
                case STATE_INIT_SUCCESS:
                case STATE_EXITING:
                case STATE_EXIT_FAILED:
                case STATE_EXIT_SUCCESS:
                case STATE_RUNNING:
                    return TRAVERSE_CONTINUE;

                case STATE_STOPPED:
                case STATE_FAILED:
                    return TRAVERSE_PRUNE;
            }
            EQUNREACHABLE;
            return TRAVERSE_PRUNE;
        }

    template< class T >
    VisitorResult _updateUp( T* entity, const std::string& name )
        {
            const uint32_t state = entity->getState() & ~STATE_DELETE;
            switch( state )
            {
                case STATE_INITIALIZING:
                case STATE_INIT_FAILED:
                case STATE_INIT_SUCCESS:
                    if( !entity->syncConfigInit( ))
                    {
                        entity->sync();
                        _error = entity->getError();
                        _result = false;
                        _sync = true;
                    }
                    else
                        entity->sync();
                    return TRAVERSE_CONTINUE;

                case STATE_EXITING:
                case STATE_EXIT_FAILED:
                case STATE_EXIT_SUCCESS:
                    if( !entity->syncConfigExit( ))
                    {
                        entity->sync();
                        _error = entity->getError();
                        _result = false;
                    }
                    else
                        entity->sync();
                    return TRAVERSE_CONTINUE;

                case STATE_RUNNING:
                case STATE_STOPPED:
                case STATE_FAILED:
                    return TRAVERSE_CONTINUE;
            }
            EQUNREACHABLE;
            return TRAVERSE_PRUNE;
        }
};

}
}
}

#endif // EQSERVER_CONFIGUPDATESYNCVISITOR_H
