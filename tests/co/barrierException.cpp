
/* Copyright (c) 2011, Cedric Stalder <cedric.stalder@gmail.com>> 
 *               2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include <pthread.h> // must come first!
#include <test.h>

#include <co/barrier.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/exception.h>
#include <co/global.h>
#include <co/init.h>
#include <co/node.h>
#include <lunchbox/sleep.h>
#include <lunchbox/rng.h>
#include <lunchbox/uuid.h>

#include <iostream>
#define EQ_TEST_RUNTIME 6000
#define NSLAVES  10

bool testNormal();
bool testException();
bool testSleep();

static uint16_t _serverPort = 0;

class BarrierThread : public lunchbox::Thread
{
public:
    BarrierThread( const uint32_t nOps, const uint32_t port ) 
        : _nOps( nOps )
        , _numExceptions( 0 )  
        , _node( new co::LocalNode )
    {
        co::ConnectionDescriptionPtr description = 
            new co::ConnectionDescription;
        description->type = co::CONNECTIONTYPE_TCPIP;
        description->port = port;
        _node->addConnectionDescription( description );

        TEST( _node->listen( ));
    }

protected:
    const uint32_t   _nOps;
    uint32_t         _numExceptions;
    co::LocalNodePtr _node;
};

class ServerThread : public BarrierThread
{
public:
    ServerThread( const uint32_t nbNode, const uint32_t nOps )
        : BarrierThread( nOps, _serverPort )
    {
        _barrier = new co::Barrier( _node, nbNode + 1 );
        _node->registerObject( _barrier );
        TEST( _barrier->isAttached( ));
    }

    /** @return the barrier unique identifier. */
    const lunchbox::UUID& getBarrierID() const { return _barrier->getID(); }
    ~ServerThread( )
    {
        _node->releaseObject( _barrier );
            
        TEST( _node->close());
        _node = 0;
        delete _barrier;
    }

    uint32_t getNumExceptions(){ return _numExceptions; };

protected:
    virtual void run()
    {
        for( uint32_t i = 0; i < _nOps; i++ )
        {
            const uint32_t timeout = co::Global::getIAttribute( 
                     co::Global::IATTR_TIMEOUT_DEFAULT );
            try
            {
                _barrier->enter( timeout );
            }
            catch( co::Exception& e )
            {
                TEST( e.getType() == co::Exception::TIMEOUT_BARRIER );
                ++_numExceptions;
            }
        }
    }
private:
    co::Barrier* _barrier;
};

class NodeThread : public BarrierThread
{
public:
    NodeThread( const lunchbox::UUID& barrierID, const uint32_t port,
                const uint32_t nOps, const uint32_t timeToSleep )
         : BarrierThread( nOps, port ) 
         , _timeToSleep( timeToSleep )
         , _barrierID( barrierID ) 
    {
        _server = new co::Node;
        co::ConnectionDescriptionPtr serverDesc = 
            new co::ConnectionDescription;
        serverDesc->port = _serverPort;
        _server->addConnectionDescription( serverDesc );
        TEST( _node->connect( _server ));

        _node->mapObject( &barrier, _barrierID );
    }

    ~NodeThread( )
    {
        _node->releaseObject( &barrier );
        _node->flushCommands();
        TEST( _node->close());
        _node = 0;
        _server = 0;
    }
    
    uint32_t getNumExceptions(){ return _numExceptions; };

protected:
    virtual void run()
    {
        for( uint32_t i = 0; i < _nOps; ++i )
        {
            lunchbox::sleep( _timeToSleep );
            const uint32_t timeout = co::Global::getIAttribute( 
                     co::Global::IATTR_TIMEOUT_DEFAULT );
            try
            {
                
                barrier.enter( timeout );
            }
            catch( co::Exception& e )
            {
                TEST( e.getType() == co::Exception::TIMEOUT_BARRIER );
                ++_numExceptions;
            }
        }
    }
            
private:
    const uint32_t _timeToSleep;
    const lunchbox::UUID& _barrierID;
    co::NodePtr _server;
    co::Barrier barrier;
};

typedef std::vector< NodeThread* > NodeThreads;  


int main( int argc, char **argv )
{
    TEST( co::init( argc, argv ));
    static lunchbox::RNG rng;
    _serverPort = (rng.get<uint16_t>() % 60000) + 1024;

    TEST( testNormal() );
    TEST( testException() );    
    TEST( testSleep() );    
    
    co::exit();
    return EXIT_SUCCESS;
}

/* the test perform no timeout */
bool testNormal()
{
    co::Global::setIAttribute( co::Global::IATTR_TIMEOUT_DEFAULT, 10000 );
    NodeThreads nodeThreads;
    nodeThreads.resize(NSLAVES);
 
    ServerThread server( NSLAVES, 1 ); 
    server.start();

    for( uint32_t i = 0; i < NSLAVES; i++ )
    {
        nodeThreads[i] = new NodeThread( server.getBarrierID(), _serverPort+i+1,
                                         1, 0 );
        nodeThreads[i]->start();
    }
    
    server.join();

    for( uint32_t i = 0; i < NSLAVES; i++ )
    {
        nodeThreads[i]->join();
        TEST( nodeThreads[i]->getNumExceptions() == 0 );    
        delete nodeThreads[i];
    }

    TEST( server.getNumExceptions() == 0 );
    return true;
}

/* the test perform no timeout */
bool testException()
{
    co::Global::setIAttribute( co::Global::IATTR_TIMEOUT_DEFAULT, 2000 );
    NodeThreads nodeThreads;
    nodeThreads.resize( NSLAVES );

    ServerThread server( NSLAVES, 1 ); 
    server.start();

    for( uint32_t i = 0; i < NSLAVES - 1; i++ )
    {
        nodeThreads[i] = new NodeThread( server.getBarrierID(), _serverPort+i+1,
                                         1, 0 );
        nodeThreads[i]->start();
    }
    
    TEST( server.join() );
    TEST( server.getNumExceptions() == 1 );
    for( uint32_t i = 0; i < NSLAVES - 1; i++ )
    {
        TEST( nodeThreads[i]->join() );
        TEST( nodeThreads[i]->getNumExceptions() == 1 );
        delete nodeThreads[i];
    }
    
    return true;
}

bool testSleep()
{
    co::Global::setIAttribute( co::Global::IATTR_TIMEOUT_DEFAULT, 2000 );
    NodeThreads nodeThreads( 5 );
    nodeThreads.resize( 5 );

    ServerThread server( 5, 1 ); 
    server.start();

    uint16_t port = _serverPort;

    nodeThreads[0] = new NodeThread( server.getBarrierID(), ++port, 5, 1000 );
    nodeThreads[0]->start();
    
    nodeThreads[1] = new NodeThread( server.getBarrierID(), ++port, 5, 1500 );
    nodeThreads[1]->start();
    
    nodeThreads[2] = new NodeThread( server.getBarrierID(), ++port, 5, 2000 );
    nodeThreads[2]->start();
    
    nodeThreads[3] = new NodeThread( server.getBarrierID(), ++port, 5, 2500 );
    nodeThreads[3]->start();

    nodeThreads[4] = new NodeThread( server.getBarrierID(), ++port, 5, 3000 );
    nodeThreads[4]->start();
    
    TEST( server.join() );

    TEST( nodeThreads[0]->join() );
    TEST( nodeThreads[1]->join() );
    TEST( nodeThreads[2]->join() );
    TEST( nodeThreads[3]->join() );
    TEST( nodeThreads[4]->join() );
    
    // Bug: if the first nodeThread is deleted before the the second nodeThread,
    // the barrier send unblock will fail because it is trying to send to an
    // unexisting connection node. Can this happen in Equalizer ?
    delete nodeThreads[0];
    delete nodeThreads[1];
    delete nodeThreads[2];
    delete nodeThreads[3];
    delete nodeThreads[4];
    
    return true;
}
