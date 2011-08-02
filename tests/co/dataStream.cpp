
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <test.h>

#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <co/base/thread.h>
#include <co/connectionDescription.h>
#include <co/command.h>
#include <co/commandCache.h>
#include <co/commandQueue.h>
#include <co/connection.h>
#include <co/init.h>
#include <co/packets.h>
#include <co/types.h>

#include <co/dataOStream.ipp>      // private impl
#include <co/base/cpuCompressor.h> // private header

// Tests the functionality of the DataOStream and DataIStream

#define CONTAINER_SIZE EQ_64KB

static std::string _message( "So long, and thanks for all the fish" );

struct DataPacket : public co::Packet
{
    DataPacket()
        {
            command        = 2;
            size           = sizeof( DataPacket ); 
            data[0]        = '\0';
        }
        
    uint64_t dataSize;
    uint32_t compressorName;
    uint32_t nChunks;
    EQ_ALIGN8( uint64_t last ); // pad and align to multiple-of-eight
    EQ_ALIGN8( uint8_t data[8] );
};

class DataOStream : public co::DataOStream
{
public:
    DataOStream() {}

protected:
    virtual void sendData( const void* buffer, const uint64_t size,
                           const bool last )
        {
            DataPacket packet;
            sendPacket( packet, buffer, size, last );
        }
};

class DataIStream : public co::DataIStream
{
public:
    void addDataCommand( co::Command& command )
        {
            TESTINFO( command->command == 2, command );
            _commands.push( command );
        }

    virtual size_t nRemainingBuffers() const { return _commands.getSize(); }
    virtual co::base::uint128_t getVersion() const { return co::VERSION_NONE;}

protected:
    virtual bool getNextBuffer( uint32_t* compressor, uint32_t* nChunks,
                                const void** chunkData, uint64_t* size )
        {
            co::Command* command = _commands.tryPop();
            if( !command )
                return false;

            TESTINFO( (*command)->command == 2, *command );

            DataPacket* packet = command->get<DataPacket>();
            
            *compressor = packet->compressorName;
            *nChunks = packet->nChunks;
            *size = packet->dataSize;
            *chunkData = packet->data;

            command->release();
            return true;
        }

private:
    co::CommandQueue _commands;
};

namespace co
{
namespace DataStreamTest
{
class Sender : public co::base::Thread
{
public:
    Sender( co::base::RefPtr< co::Connection > connection )
            : Thread(),
              _connection( connection )
        {}
    virtual ~Sender(){}

protected:
    virtual void run()
        {
            ::DataOStream stream;

            stream._connections.push_back( _connection );
            stream._enable();

            int foo = 42;
            stream << foo;
            stream << 43.0f;
            stream << 44.0;

            std::vector< double > doubles;
            for( size_t i=0; i<CONTAINER_SIZE; ++i )
                doubles.push_back( static_cast< double >( i ));

            stream << doubles;
            stream << _message;

            stream.disable();
        }

private:
    co::base::RefPtr< co::Connection > _connection;
};
}
}

int main( int argc, char **argv )
{
    co::init( argc, argv );
    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_PIPE;
    co::ConnectionPtr connection = co::Connection::create( desc );

    TEST( connection->connect( ));
    co::DataStreamTest::Sender sender( connection->acceptSync( ));
    TEST( sender.start( ));

    ::DataIStream stream;
    co::CommandCache commandCache;
    bool receiving = true;

    while( receiving )
    {
        uint64_t size;
        connection->recvNB( &size, sizeof( size ));
        TEST( connection->recvSync( 0, 0 ));
        TEST( size );

        co::Command& command = commandCache.alloc( 0, 0, size );
        size -= sizeof( size );

        char* ptr = reinterpret_cast< char* >( command.get< co::Packet >( )) +
                                                        sizeof( size );
        connection->recvNB( ptr, size );
        TEST( connection->recvSync( 0, 0 ) );
        TEST( command.isValid( ));
        
        switch( command->command )
        {
            case 2:
                stream.addDataCommand( command );
                TEST( !command.isFree( ));
                receiving = !command.get< DataPacket >()->last;
                break;
            default:
                TEST( false );
        }
    }

    int foo;
    stream >> foo;
    TESTINFO( foo == 42, foo );

    float fFoo;
    stream >> fFoo;
    TEST( fFoo == 43.f );

    double dFoo;
    stream >> dFoo;
    TEST( dFoo == 44.0 );

    std::vector< double > doubles;
    stream >> doubles;
    for( size_t i=0; i<CONTAINER_SIZE; ++i )
        TEST( doubles[i] == static_cast< double >( i ));

    std::string message;
    stream >> message;
    TEST( message.length() == _message.length() );
    TESTINFO( message == _message, 
              '\'' <<  message << "' != '" << _message << '\'' );

    TEST( sender.join( ));
    connection->close();
    return EXIT_SUCCESS;
}
