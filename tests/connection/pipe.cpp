
#include <eq/base/thread.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/pipeConnection.h>

#include <alloca.h>
#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

class Server : public eqBase::Thread
{
public:
    void start( RefPtr<Connection> connection )
        {
            _connection = connection;
            eqBase::Thread::start();
        }

protected:
    virtual void* run()
        {
            if( !_connection || 
                _connection->getState() != Connection::STATE_CONNECTED )

                return (void*)EXIT_FAILURE;

            cerr << "Server up" << endl;
            char c;
            while( _connection->recv( &c, 1 ))
            {
                cerr << "Server recv: " << c << endl;
                _connection->send( &c, 1 );
            }
            _connection->close();
            _connection = NULL;
            return EXIT_SUCCESS;
        }
private:
    RefPtr<Connection> _connection;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    PipeConnection* pipeConnection = new PipeConnection();
    RefPtr<Connection>  connection = pipeConnection;
    if( !connection->connect( ))
        exit( EXIT_FAILURE );

    Server server;
    server.start( pipeConnection->getChildEnd( ));

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    connection->send( message, nChars );
    connection->recv( response, nChars );
    cerr << "Client recv: " << response << endl;

    connection->close();
    connection = NULL;

    server.join();
    
    return EXIT_SUCCESS;
}
