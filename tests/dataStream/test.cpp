
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include <eq/base/thread.h>
#include <eq/net/command.h>
#include <eq/net/commandQueue.h>
#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/packets.h>

using namespace std;

// Tests the functionality of the DataOStream and DataIStream

#define CONTAINER_SIZE 4096

struct HeaderPacket : public eqNet::Packet
{
    HeaderPacket()
        {
            command        = 0;
            size           = sizeof( HeaderPacket ); 
        }
};

struct DataPacket : public eqNet::Packet
{
    DataPacket()
        {
            command        = 2;
            size           = sizeof( DataPacket ); 
            data[0]        = '\0';
        }
        
    uint64_t dataSize;
    EQ_ALIGN8( char data[8] );
};

struct FooterPacket : public eqNet::Packet
{
    FooterPacket()
        {
            command        = 4;
            size           = sizeof( FooterPacket ); 
        }
};

class DataOStream : public eqNet::DataOStream
{
protected:
    virtual void sendHeader()
        {
            HeaderPacket packet;
            eqNet::Connection::send( _connections, packet, true /*isLocked*/ );
        }

    virtual void sendBuffer( const void* buffer, const uint64_t size )
        {
            DataPacket packet;
            packet.dataSize = size;
            eqNet::Connection::send( _connections, packet, buffer, size, 
                                     true /*isLocked*/ );
        }

    virtual void sendFooter()
        {
            FooterPacket packet;
            eqNet::Connection::send( _connections, packet, true /*isLocked*/ );
        }
};

class DataIStream : public eqNet::DataIStream
{
public:
    void addDataCommand( eqNet::Command& command )
        {
            TESTINFO( command->command == 2, command );
            _commands.push( command );
        }

protected:
    virtual bool getNextBuffer( const void** buffer, uint64_t* size )
        {
            eqNet::Command* command = _commands.tryPop();
            if( !command )
                return false;

            TESTINFO( (*command)->command == 3, *command );

            DataPacket* packet = command->getPacket<DataPacket>();
            
            *buffer = packet->data;
            *size   = packet->dataSize;
            return true;
        }

private:
    eqNet::CommandQueue _commands;
};


class Sender : public eqBase::Thread
{
public:
    Sender( eqBase::RefPtr< eqNet::Connection > connection )
            : Thread(),
              _connection( connection )
        {}
    virtual ~Sender(){}

protected:
    virtual void* run()
        {
            DataOStream             stream;
            eqNet::ConnectionVector connections;

            connections.push_back( _connection );
            stream.enable( connections );

            int foo = 42;
            stream << foo;

            vector< double > doubles;
            for( size_t i=0; i<CONTAINER_SIZE; ++i )
                doubles.push_back( static_cast< double >( i ));
            stream << doubles;

            stream.disable();
            return EXIT_SUCCESS;
        }

private:
    eqBase::RefPtr< eqNet::Connection > _connection;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    eqBase::RefPtr< eqNet::Connection > connection = 
        eqNet::Connection::create( eqNet::CONNECTIONTYPE_PIPE );

    TEST( connection->connect( ));
    Sender sender( connection );
    TEST( sender.start( ));

    DataIStream     stream;
    eqNet::Command  command;
    bool            receiving = true;

    while( receiving )
    {
        uint64_t   size;
        const bool gotSize = connection->recv( &size, sizeof( size ));
        TEST( gotSize );
        TEST( size );

        command.allocate( 0, 0, size );
        size -= sizeof( size );

        char*      ptr     = reinterpret_cast< char* >( command.getPacket( )) +
                                                        sizeof( size );
        const bool gotData = connection->recv( ptr, size );

        TEST( gotData );
        TEST( command.isValid( ));
        
        switch( command->command )
        {
            case 0: // header, nop
                break;
            case 2:
                stream.addDataCommand( command );
                break;
            case 4:
                receiving = false;
                break;
            default:
                TEST( false );
        }
    }

    int foo;
    stream >> foo;
    TEST( foo == 42 );

    vector< double > doubles;
    stream >> doubles;
    for( size_t i=0; i<CONTAINER_SIZE; ++i )
        TEST( doubles[i] == static_cast< double >( i ));

    TEST( sender.join( ));
    connection->close();
    return EXIT_SUCCESS;
}
