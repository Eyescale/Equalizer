
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

#ifndef EQSERVER_CONFIGUPDATEVISITOR_H
#define EQSERVER_CONFIGUPDATEVISITOR_H

#include "configVisitor.h" // base class

namespace eq
{
namespace server
{
namespace
{

class ConfigUpdateVisitor : public ConfigVisitor
{
public:
    ConfigUpdateVisitor( const uint128_t& initID, const uint32_t frameNumber )
            : _initID( initID ), _frameNumber( frameNumber ) {}
    virtual ~ConfigUpdateVisitor() {}

    virtual VisitorResult visitPre( Node* node )
        { return _updateDown( node ); }
    virtual VisitorResult visitPost( Node* node )
        {
            const VisitorResult result = _updateUp( node );
            if( result == TRAVERSE_CONTINUE )
                node->flushSendBuffer();
            return result;
        }

    virtual VisitorResult visitPre( Pipe* pipe )
        { return _updateDown( pipe ); }
    virtual VisitorResult visitPost( Pipe* pipe )
        { return _updateUp( pipe ); }

    virtual VisitorResult visitPre( Window* window )
        { return _updateDown( window ); }
    virtual VisitorResult visitPost( Window* window )
        { return _updateUp( window ); }

    virtual VisitorResult visit( Channel* channel )
        {
            if( _updateDown( channel ) == TRAVERSE_CONTINUE )
                return _updateUp( channel );
            return TRAVERSE_CONTINUE;
        }

private:
    const uint128_t _initID;
    const uint32_t  _frameNumber;

    template< class T > VisitorResult _updateDown( T* entity ) const
        {
            const uint32_t state = entity->getState() & ~STATE_DELETE;

            switch( state )
            {
                case STATE_STOPPED:
                    if( entity->isActive( ))
                    {
                        entity->setError( ERROR_NONE );
                        entity->configInit( _initID, _frameNumber );
                        return TRAVERSE_CONTINUE;
                    }
                    return TRAVERSE_PRUNE;

                case STATE_RUNNING:
                    return TRAVERSE_CONTINUE;
                case STATE_FAILED:
                    if( entity->isActive( ))
                        return TRAVERSE_PRUNE;
                    return TRAVERSE_CONTINUE;
            }
            EQUNREACHABLE;
            return TRAVERSE_PRUNE;
        }

    template< class T > VisitorResult _updateUp( T* entity ) const
        {
            const uint32_t state = entity->getState() & ~STATE_DELETE;
            switch( state )
            {
                case STATE_INITIALIZING:
                case STATE_INIT_FAILED:
                case STATE_INIT_SUCCESS:
                    return TRAVERSE_CONTINUE;

                case STATE_RUNNING:
                    if( !entity->isActive( ))
                        entity->configExit();
                    return TRAVERSE_CONTINUE;

                case STATE_FAILED:
                    EQASSERT( !entity->isActive( ));
                    if( !entity->isActive( ))
                    {
                        entity->setState( STATE_STOPPED );
                        return TRAVERSE_PRUNE;
                    }
                    return TRAVERSE_CONTINUE;
            }
            EQASSERTINFO( false, State( state ));
            return TRAVERSE_PRUNE;
        }
};

}
}
}

#endif // EQSERVER_CONFIGUPDATEVISITOR_H
