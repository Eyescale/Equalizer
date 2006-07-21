
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

%{
#include "loader.h"

#include "compound.h"
#include "global.h"
#include "pipe.h"
#include "server.h"
#include "swapBarrier.h"
#include "window.h"

#include <eq/base/base.h>
#include <string>

    namespace eqLoader
    {
        static eqs::Loader* loader = NULL;
        static std::string  stringBuf;
        
        static eqs::Server*      server;
        static eqs::Config*      config;
        static eqs::Node*        node;
        static eqs::Pipe*        eqPipe; // avoid name clash with pipe()
        static eqs::Window*      window;
        static eqs::Channel*     channel;
        static eqs::Compound*    compound;
        static eqs::SwapBarrier* swapBarrier;
        static eqBase::RefPtr<eqNet::ConnectionDescription> 
            connectionDescription;
    }

    using namespace std;
    using namespace eqLoader;
    using namespace eqBase;

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
%token EQTOKEN_APPNODE
%token EQTOKEN_NODE
%token EQTOKEN_PIPE
%token EQTOKEN_WINDOW
%token EQTOKEN_CHANNEL
%token EQTOKEN_COMPOUND
%token EQTOKEN_CONNECTION
%token EQTOKEN_NAME
%token EQTOKEN_TYPE
%token EQTOKEN_TCPIP
%token EQTOKEN_HOSTNAME
%token EQTOKEN_COMMAND
%token EQTOKEN_TIMEOUT
%token EQTOKEN_TASK
%token EQTOKEN_CLEAR
%token EQTOKEN_DRAW
%token EQTOKEN_VIEWPORT
%token EQTOKEN_RANGE
%token EQTOKEN_DISPLAY
%token EQTOKEN_WALL
%token EQTOKEN_BOTTOM_LEFT
%token EQTOKEN_BOTTOM_RIGHT
%token EQTOKEN_TOP_LEFT
%token EQTOKEN_SYNC
%token EQTOKEN_LATENCY
%token EQTOKEN_SWAPBARRIER

%token EQTOKEN_STRING
%token EQTOKEN_FLOAT
%token EQTOKEN_INTEGER
%token EQTOKEN_UNSIGNED

%union{
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
}

%type <_string>         STRING;
%type <_int>            INTEGER;
%type <_unsigned>       UNSIGNED;
%type <_connectionType> connectionType;
%type <_viewport>       viewport;
%type <_float>          FLOAT;

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
     | EQTOKEN_CONNECTION_TCPIP_PORT UNSIGNED
     {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_LAUNCH_TIMEOUT UNSIGNED
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
        configAttributes
        nodes compounds '}' { server->addConfig( config ); config = NULL; }
configAttributes: /*null*/ | configAttribute | configAttributes configAttribute
configAttribute:
    EQTOKEN_LATENCY UNSIGNED  { config->setLatency( $2 ); }

nodes: node | nodes node
node: appNode | otherNode
otherNode: EQTOKEN_NODE '{' { node = loader->createNode(); }
               connections
               nodeAttributes
               pipes '}' { config->addNode( node ); node = NULL; }
appNode: EQTOKEN_APPNODE '{' { node = loader->createNode(); }
            connections
            nodeAttributes
            pipes '}' { config->addApplicationNode( node ); node = NULL; }
nodeAttributes: /*null*/ | nodeAttribute | nodeAttributes nodeAttribute
nodeAttribute: /*TODO*/

connections: /*null*/ 
             { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             }
             | connection | connections connection
connection: EQTOKEN_CONNECTION 
            '{' { connectionDescription = new eqs::ConnectionDescription(); }
            connectionAttributes '}' 
             { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             }
connectionAttributes: /*null*/ | connectionAttribute | 
                      connectionAttributes connectionAttribute
connectionAttribute:
    EQTOKEN_TYPE connectionType { connectionDescription->type = $2; }
    | EQTOKEN_HOSTNAME STRING   { connectionDescription->hostname = $2; }
    | EQTOKEN_COMMAND STRING    { connectionDescription->launchCommand = $2; }
    | EQTOKEN_TIMEOUT UNSIGNED  { connectionDescription->launchTimeout = $2; }


pipes: pipe | pipes pipe
pipe: EQTOKEN_PIPE '{' { eqPipe = loader->createPipe(); }
        pipeAttributes
        windows '}' { node->addPipe( eqPipe ); eqPipe = NULL; }
pipeAttributes: /*null*/ | pipeAttribute | pipeAttributes pipeAttribute
pipeAttribute:
    EQTOKEN_DISPLAY UNSIGNED         { eqPipe->setDisplay( $2 ); }
    | EQTOKEN_VIEWPORT viewport 
        {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)$2[0], (int)$2[1],
                                                      (int)$2[2], (int)$2[3] ));
        }

windows: window | windows window
window: EQTOKEN_WINDOW '{' { window = loader->createWindow(); }
        windowAttributes
        channels '}' { eqPipe->addWindow( window ); window = NULL; }
windowAttributes: /*null*/ | windowAttribute | windowAttributes windowAttribute
windowAttribute: 
    EQTOKEN_NAME STRING              { window->setName( $2 ); }
    | EQTOKEN_VIEWPORT viewport
        {
            if( $2[2] > 1 || $2[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)$2[0], 
                                          (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                window->setViewport( eq::Viewport($2[0], $2[1], $2[2], $2[3])); 
        }

channels: channel | channels channel
channel: EQTOKEN_CHANNEL '{' { channel = loader->createChannel(); }
         channelAttributes
        '}' { window->addChannel( channel ); channel = NULL; }
channelAttributes:
     /*null*/ | channelAttribute | channelAttributes channelAttribute
channelAttribute: 
    EQTOKEN_NAME STRING { channel->setName( $2 ); }
    | EQTOKEN_VIEWPORT viewport
        {
            if( $2[2] > 1 || $2[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)$2[0],
                                          (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                channel->setViewport(eq::Viewport( $2[0], $2[1], $2[2], $2[3]));
        }


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
    EQTOKEN_NAME STRING { compound->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
    {
         eqs::Channel* channel = config->findChannel( $2 );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             compound->setChannel( channel );
    }
    | EQTOKEN_TASK '['      { compound->setTasks( eqs::Compound::TASK_NONE ); }
        compoundTasks ']'
    | EQTOKEN_VIEWPORT viewport
        { compound->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] )); }
    | EQTOKEN_RANGE '[' FLOAT FLOAT ']'
        { compound->setRange( eq::Range( $3, $4 )); }
    | wall
    | swapBarrier

compoundTasks: /*null*/ | compoundTask | compoundTasks compoundTask
compoundTask:
    EQTOKEN_CLEAR   { compound->enableTasks( eqs::Compound::TASK_CLEAR ); }
    | EQTOKEN_DRAW  { compound->enableTasks( eqs::Compound::TASK_DRAW ); }

wall: EQTOKEN_WALL '{'
          EQTOKEN_BOTTOM_LEFT  '[' FLOAT FLOAT FLOAT ']' 
          EQTOKEN_BOTTOM_RIGHT '[' FLOAT FLOAT FLOAT ']' 
          EQTOKEN_TOP_LEFT     '[' FLOAT FLOAT FLOAT ']' 
      '}'
    { 
        eq::Wall wall;
        wall.bottomLeft[0] = $5;
        wall.bottomLeft[1] = $6;
        wall.bottomLeft[2] = $7;

        wall.bottomRight[0] = $11;
        wall.bottomRight[1] = $12;
        wall.bottomRight[2] = $13;

        wall.topLeft[0] = $17;
        wall.topLeft[1] = $18;
        wall.topLeft[2] = $19;
        compound->setWall( wall );
    }

swapBarrier : EQTOKEN_SWAPBARRIER '{' { swapBarrier = new eqs::SwapBarrier(); }
    swapBarrierAttributes '}'
        { 
            compound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        } 
swapBarrierAttributes: /*null*/ | swapBarrierAttribute 
    | swapBarrierAttributes swapBarrierAttribute
swapBarrierAttribute: 
    EQTOKEN_NAME STRING { swapBarrier->setName( $2 ); }

viewport: '[' FLOAT FLOAT FLOAT FLOAT ']'
     { 
         $$[0] = $2;
         $$[1] = $3;
         $$[2] = $4;
         $$[3] = $5;
     }

STRING: EQTOKEN_STRING
     {
         stringBuf = yytext;
         $$ = stringBuf.c_str(); 
     }
FLOAT: EQTOKEN_FLOAT                       { $$ = atof( yytext ); }
    | INTEGER                              { $$ = $1; }

INTEGER: EQTOKEN_INTEGER                   { $$ = atoi( yytext ); }
    | UNSIGNED                             { $$ = $1; }
UNSIGNED: EQTOKEN_UNSIGNED                 { $$ = atoi( yytext ); }
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
