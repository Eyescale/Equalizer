
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

%{
#include "loader.h"

#include "compound.h"
#include "global.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

#include <eq/base/base.h>
#include <string>

    namespace eqLoader
    {
        static eqs::Loader* loader = NULL;
        static std::string  stringBuf;
        
        static eqs::Server*   server;
        static eqs::Config*   config;
        static eqs::Node*     node;
        static eqs::Pipe*     eqPipe; // avoid name clash with pipe()
        static eqs::Window*   window;
        static eqs::Channel*  channel;
        static eqs::Compound* compound;
        static eqBase::RefPtr<eqs::ConnectionDescription> connectionDescription;
    }

    using namespace std;
    using namespace eqLoader;

    int eqLoader_lex();

    #define yylineno eqLoader_lineno
    void yyerror( char *errmsg );
    extern char* yytext;
    extern FILE* yyin;
    extern int yylineno;
%}

%token EQTOKEN_GLOBAL
%token EQTOKEN_CONNECTION_TYPE
%token EQTOKEN_CONNECTION_HOSTNAME
%token EQTOKEN_CONNECTION_TCPIP_PORT
%token EQTOKEN_CONNECTION_LAUNCH_TIMEOUT
%token EQTOKEN_CONNECTION_LAUNCH_COMMAND
%token EQTOKEN_SERVER
%token EQTOKEN_CONFIG
%token EQTOKEN_NODE
%token EQTOKEN_PIPE
%token EQTOKEN_WINDOW
%token EQTOKEN_CHANNEL
%token EQTOKEN_COMPOUND
%token EQTOKEN_CONNECTION
%token EQTOKEN_NAME
%token EQTOKEN_MODE
%token EQTOKEN_TYPE
%token EQTOKEN_TCPIP
%token EQTOKEN_SYNC

%token EQTOKEN_STRING
%token EQTOKEN_INTEGER

%union{
    const char*         stringval;
    int                 integer;
    float               real;
    bool                boolean;
    eqs::Compound::Mode _compoundMode;
}

%type <stringval>     STRING;
%type <integer>       INTEGER connectionType;
%type <_compoundMode> compoundMode;
//%type <real>          FLOATVAL;
//%type <boolean>       BOOL;

%%

file:   global server;

global: EQTOKEN_GLOBAL '{' globals '}' 
        |
        ;

globals: global | globals global;

global:
     EQTOKEN_CONNECTION_TYPE connectionType 
     { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, $2 ); 
     }
     | EQTOKEN_CONNECTION_HOSTNAME STRING
     {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, $2 );
     }
     | EQTOKEN_CONNECTION_TCPIP_PORT INTEGER
     {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_LAUNCH_TIMEOUT INTEGER
     {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, $2 );
     }
     | EQTOKEN_CONNECTION_LAUNCH_COMMAND STRING
     {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, $2 );
     };

connectionType: EQTOKEN_TCPIP { $$ = eqNet::Connection::TYPE_TCPIP; };

server: EQTOKEN_SERVER '{' { server = loader->createServer(); }
        configs '}'

configs: config | configs config
config: EQTOKEN_CONFIG '{' { config = loader->createConfig(); }
        nodes compounds '}' { server->addConfig( config ); config = NULL; }

nodes: node | nodes node
node: EQTOKEN_NODE '{' { node = loader->createNode(); }
        pipes '}' { config->addNode( node ); node = NULL; }

pipes: pipe | pipes pipe
pipe: EQTOKEN_PIPE '{' { eqPipe = loader->createPipe(); }
        windows '}' { node->addPipe( eqPipe ); eqPipe = NULL; }

windows: window | windows window
window: EQTOKEN_WINDOW '{' { window = loader->createWindow(); }
        channels '}' { eqPipe->addWindow( window ); window = NULL; }

channels: channel | channels channel
channel: EQTOKEN_CHANNEL '{' { channel = loader->createChannel(); }
         channelName
        '}' { window->addChannel( channel ); channel = NULL; }
channelName: /*null*/ | EQTOKEN_NAME STRING { channel->setName( $2 ); }

compounds: compound | compounds compound
compound: EQTOKEN_COMPOUND '{' 
              {
                  eqs::Compound* child = loader->createCompound();
                  if( compound )
                      compound->addChild( child );
                  else
                      config->addCompound( child );
                  compound = child;
              }
          compoundAttributes 
          compoundChildren '}' { compound = compound->getParent(); } 
compoundChildren: /*null*/ | compounds

compoundAttributes: /*null*/ | compoundAttribute |
                    compoundAttributes compoundAttribute
compoundAttribute: 
    EQTOKEN_MODE '[' compoundMode ']' { compound->setMode( $3 ); } |
    EQTOKEN_CHANNEL STRING
    {
         eqs::Channel* channel = config->findChannel( $2 );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             compound->setChannel( channel );
    }

compoundMode:
    EQTOKEN_SYNC { $$ = eqs::Compound::MODE_SYNC; }

INTEGER: EQTOKEN_INTEGER      { $$ = atoi(yytext); }
STRING:  EQTOKEN_STRING
     {
         stringBuf = yytext;
         $$ = stringBuf.c_str(); 
     }
%%

void yyerror( char *errmsg )
{
    EQERROR << "Parse error: '" << errmsg << "', line " << yylineno
            << " at '" << yytext << "'" << endl;
}

//---------------------------------------------------------------------------
// loader
//---------------------------------------------------------------------------
eqs::Server* eqs::Loader::loadConfig( const string& filename )
{
    EQASSERTINFO( !loader, "Config file loader is not reentrant" );
    loader = this;

    yyin = fopen( filename.c_str(), "r" );
    if( !yyin )
    {
        EQERROR << "Can't open config file " << filename << endl;
        return NULL;
    }

    server      = NULL;
    bool retval = true;
    if( eqLoader_parse() )
        retval = false;

    fclose( yyin );
    loader = NULL;
    return server;
}
