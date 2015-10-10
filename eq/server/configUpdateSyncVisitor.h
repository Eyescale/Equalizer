
/* Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
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
    ConfigUpdateSyncVisitor()
        : _runningChannels( 0 )
        , _failure( false )
        , _sync( false )
    {}
    virtual ~ConfigUpdateSyncVisitor() {}

    VisitorResult visitPre( Config* ) override
    {
        _runningChannels = 0;
        _failure = false;
        _sync = false;
        return TRAVERSE_CONTINUE;
    }

    VisitorResult visitPre( Node* node ) override
        { return _updateDown( node ); }
    VisitorResult visitPost( Node* node ) override
    {
        const VisitorResult& result = _updateUp( node );
        node->flushSendBuffer();
        return result;
    }

    VisitorResult visitPre( Pipe* pipe ) override
        { return _updateDown( pipe ); }
    VisitorResult visitPost( Pipe* pipe ) override
        { return _updateUp( pipe ); }

    VisitorResult visitPre( Window* window ) override
        { return _updateDown( window ); }
    VisitorResult visitPost( Window* window ) override
        { return _updateUp( window ); }

    VisitorResult visit( Channel* channel ) override
    {
        const VisitorResult result = _updateUp( channel );
        if( channel->isRunning( ))
            ++_runningChannels;
        return result;
    }

    size_t getNumRunningChannels() const { return _runningChannels; }
    bool hadFailure() const { return _failure; }
    bool needsSync() const { return _sync; }

private:
    size_t _runningChannels;
    bool _failure;
    bool _sync;   // call again after init failure

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
        LBUNREACHABLE;
        return TRAVERSE_PRUNE;
    }

    template< class T > VisitorResult _updateUp( T* entity )
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
                    _failure = true;
                    _sync = true;
                    LBWARN << lunchbox::className( entity ) << " init failed"
                           << std::endl;
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
                    _failure = true;
                    LBWARN << lunchbox::className( entity ) << " exit failed"
                           << std::endl;
                }
                else
                    entity->sync();
                return TRAVERSE_CONTINUE;

            case STATE_RUNNING:
            case STATE_STOPPED:
            case STATE_FAILED:
                return TRAVERSE_CONTINUE;
        }
        LBUNREACHABLE;
        return TRAVERSE_PRUNE;
    }
};

}
}
}

#endif // EQSERVER_CONFIGUPDATESYNCVISITOR_H
