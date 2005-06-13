
#include <connection.h>

using namespace eqNet;

int main( int argc, char **argv )
{
    Connection *conn = new Connection();
    conn->connect( "localhost:4242" );

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    conn->write( message, nChars );
    conn->read( response, nChars );
    fprintf( stderr, "%s\n", response );
    conn->close();

    return 0;
}
