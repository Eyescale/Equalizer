
/* Copyright (c) 2011-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011, Carsten Rohn <carsten.rohn@rtt.ag>
 *               2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "tileEqualizer.h"

#include "../compound.h"
#include "../compoundVisitor.h"
#include "../config.h"
#include "../server.h"
#include "../tileQueue.h"
#include "../view.h"

namespace eq
{
namespace server
{
namespace
{

TileQueue* _findQueue( const std::string& name, const TileQueues& queues )
{
    for( TileQueuesCIter i = queues.begin(); i != queues.end(); ++i )
    {
        if ((*i)->getName() == name)
            return *i;
    }
    return 0;
}

class InputQueueCreator : public CompoundVisitor
{
public:
    InputQueueCreator( const eq::fabric::Vector2i& size,
                       const std::string& name )
        : CompoundVisitor()
        , _tileSize( size )
        , _name( name )
    {}

    /** Visit a leaf compound. */
    virtual VisitorResult visitLeaf( Compound* compound )
    {
        if( _findQueue( _name, compound->getInputTileQueues( )))
            return TRAVERSE_CONTINUE;

        // reset compound viewport to (0, 0, 1, 1) (#108)
        if( !compound->getViewport().hasArea() )
            compound->setViewport( Viewport( ));

        TileQueue* input = new TileQueue;
        ServerPtr server = compound->getServer();
        server->registerObject( input );
        input->setTileSize( _tileSize );
        input->setName( _name );
        input->setAutoObsolete( compound->getConfig()->getLatency( ));

        compound->addInputTileQueue( input );
        return TRAVERSE_CONTINUE;
    }

private:
    const eq::fabric::Vector2i& _tileSize;
    const std::string& _name;
};

class InputQueueDestroyer : public CompoundVisitor
{
public:
    explicit InputQueueDestroyer( const std::string& name )
        : CompoundVisitor()
        , _name( name )
    {
    }

    /** Visit a leaf compound. */
    virtual VisitorResult visitLeaf( Compound* compound )
    {
        TileQueue* q = _findQueue( _name, compound->getInputTileQueues( ));
        if( q )
        {
            compound->removeInputTileQueue( q );
            ServerPtr server = compound->getServer();
            q->flush();
            server->deregisterObject( q );
            delete q;
        }

        return TRAVERSE_CONTINUE;
    }
private:
    const std::string& _name;
};

}

TileEqualizer::TileEqualizer()
    : Equalizer()
    , _created( false )
    , _name( "TileEqualizer" )
{
}

TileEqualizer::TileEqualizer( const TileEqualizer& from )
    : Equalizer( from )
    , _created( from._created )
    , _name( from._name )
{
}

std::string TileEqualizer::_getQueueName() const
{
    std::ostringstream name;
    name << "queue." << _name << (void*)this;
    return name.str();
}

void TileEqualizer::_createQueues( Compound* compound )
{
    _created = true;
    const std::string& name = _getQueueName();
    if( !_findQueue( name, compound->getOutputTileQueues( )))
    {
        TileQueue* output = new TileQueue;
        ServerPtr server = compound->getServer();
        server->registerObject( output );
        output->setTileSize( getTileSize( ));
        output->setName( name );
        output->setAutoObsolete( compound->getConfig()->getLatency( ));

        compound->addOutputTileQueue( output );
    }

    InputQueueCreator creator( getTileSize(), name );
    compound->accept( creator );
}

void TileEqualizer::_destroyQueues( Compound* compound )
{
    const std::string& name = _getQueueName();
    TileQueue* q = _findQueue( name, compound->getOutputTileQueues() );
    if ( q )
    {
        compound->removeOutputTileQueue( q );
        ServerPtr server = compound->getServer();
        q->flush();
        server->deregisterObject( q );
        delete q;
    }

    InputQueueDestroyer destroyer( name );
    compound->accept( destroyer );
    _created = false;
}

void TileEqualizer::notifyUpdatePre( Compound* compound,
                                     const uint32_t /*frame*/ )
{
    if( isActive() && !_created )
        _createQueues( compound );

    if( !isActive() && _created )
        _destroyQueues( compound );
}

std::ostream& operator << ( std::ostream& os, const TileEqualizer* lb )
{
    if( lb )
    {
        os << lunchbox::disableFlush
           << "tile_equalizer" << std::endl
           << "{" << std::endl
           << "    name \"" << lb->getName() << "\"" << std::endl
           << "    size " << lb->getTileSize() << std::endl
           << "}" << std::endl << lunchbox::enableFlush;
    }
    return os;
}


} //server
} //eq
