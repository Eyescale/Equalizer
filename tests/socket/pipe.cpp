
#include <eq/base/thread.h>
#include <eq/net/connection.h>
#include <eq/net/pipeConnection.h>

#include <alloca.h>
#include <iostream>

using namespace eqNet;
using namespace std;

class Server : public eqBase::Thread
{
public:
    void start( Connection* connection )
        {
            _connection = connection;
            eqBase::Thread::start();
        }

protected:
    virtual ssize_t run()
        {
            if( !_connection || 
                _connection->getState() != Connection::STATE_CONNECTED )
                exit( EXIT_FAILURE );

            cerr << "Server up" << endl;
            char c;
            while( _connection->recv( &c, 1 ))
            {
                cerr << "Server recv: " << c << endl;
                _connection->send( &c, 1 );
            }
            _connection->close();
            return EXIT_SUCCESS;
        }
private:
    Connection* _connection;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    PipeConnection *connection = (PipeConnection*)Connection::create(TYPE_PIPE);

    ConnectionDescription connDesc;
    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    Server server;
    server.start( connection->getChildEnd( ));

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    connection->send( message, nChars );
    connection->recv( response, nChars );
    cerr << "Client recv: " << response << endl;

    connection->close();

    return EXIT_SUCCESS;
}
