
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Carsten Rohn <carsten.rohn@rtt.ag>
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

#include "types.h"
#include "compound.h"
#include "config.h"
#include "tileQueue.h"
#include "compoundVisitor.h"
#include "server.h"
#include "view.h"

#include "tileEqualizer.h"

namespace
{
using namespace eq::server;

TileQueue* searchForName( const std::string& name, const TileQueues& queues )
{
    for (TileQueuesCIter i = queues.begin(); i != queues.end(); ++i )
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
    {
        _tileSize = size;
        _name = name;
    }

    /** Visit a leaf compound. */
    virtual VisitorResult visitLeaf( Compound* compound )
    {
        if ( searchForName( _name, compound->getInputTileQueues()))
            return TRAVERSE_CONTINUE;

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
    eq::fabric::Vector2i _tileSize;
    std::string _name;
};

class InputQueueDestroyer : public CompoundVisitor
{
public:
    InputQueueDestroyer( const std::string& name )
        : CompoundVisitor()
    {
        _name = name;
    }

    /** Visit a leaf compound. */
    virtual VisitorResult visitLeaf( Compound* compound )
    {
        TileQueue* q = searchForName( _name, compound->getInputTileQueues());
        if ( q )
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
    std::string _name;
};

}

namespace eq
{
namespace server
{

TileEqualizer::TileEqualizer()
    : Equalizer()
    , _created( false )
    , _size( 64, 64 )
    , _name( "tileEQ" )
{
}

TileEqualizer::TileEqualizer( const TileEqualizer& from )
    : Equalizer( from )
    , _created( from._created )
    , _size( from._size )
    , _name( from._name )
{
}

void TileEqualizer::_createQueues( Compound* compound )
{
    if ( !searchForName( _name, compound->getOutputTileQueues()))
    {
        TileQueue* output = new TileQueue;
        ServerPtr server = compound->getServer();
        server->registerObject( output );
        output->setTileSize( _size );
        output->setName( _name );
        output->setAutoObsolete( compound->getConfig()->getLatency( ));

        compound->addOutputTileQueue( output );
    }

    InputQueueCreator creator( _size, _name );
    compound->accept( creator );

    _created = true;
}

void TileEqualizer::_destroyQueues( Compound* compound )
{
    TileQueue* q = searchForName( _name, compound->getOutputTileQueues() );
    if ( q )
    {
        compound->removeOutputTileQueue( q );
        ServerPtr server = compound->getServer();
        q->flush();
        server->deregisterObject( q );
        delete q;
    }

    InputQueueDestroyer destroyer( _name );
    compound->accept( destroyer );

    _created = false;
}

void TileEqualizer::notifyUpdatePre( Compound* compound, 
                                     const uint32_t frameNumber )
{
    if ( isActivated() && !_created )
        _createQueues( compound );
    
    if ( !isActivated() && _created )
        _destroyQueues( compound );
}

std::ostream& operator << ( std::ostream& os, const TileEqualizer* lb )
{
    if( lb )
    {
        os << co::base::disableFlush;
        os << "tile_equalizer" << std::endl << "{" << std::endl;
        os << "    name \"" << lb->getName() << "\"" << std::endl;
        os << "    size [ " << lb->getTileSize().x() << " ";
        os << lb->getTileSize().y() << " ]" << std::endl << "}" << std::endl;
        os << co::base::enableFlush;
    }

    return os;
}


} //server
} //eq
