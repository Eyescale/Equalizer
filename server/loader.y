
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

%{
#include "loader.h"

#include "compound.h"
#include "frame.h"
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
        static eqs::Compound*    eqCompound; // avoid name clash on Darwin
        static eqs::SwapBarrier* swapBarrier;
        static eqs::Frame*       frame;
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
%token EQTOKEN_CONNECTION_SATTR_HOSTNAME
%token EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND
%token EQTOKEN_CONNECTION_IATTR_TYPE
%token EQTOKEN_CONNECTION_IATTR_TCPIP_PORT
%token EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT
%token EQTOKEN_CONFIG_FATTR_EYE_BASE
%token EQTOKEN_WINDOW_IATTR_HINTS_STEREO
%token EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER
%token EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN
%token EQTOKEN_WINDOW_IATTR_HINTS_DECORATION
%token EQTOKEN_WINDOW_IATTR_PLANES_COLOR
%token EQTOKEN_WINDOW_IATTR_PLANES_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_DEPTH
%token EQTOKEN_WINDOW_IATTR_PLANES_STENCIL
%token EQTOKEN_SERVER
%token EQTOKEN_CONFIG
%token EQTOKEN_APPNODE
%token EQTOKEN_NODE
%token EQTOKEN_PIPE
%token EQTOKEN_WINDOW
%token EQTOKEN_ATTRIBUTES
%token EQTOKEN_HINTS
%token EQTOKEN_STEREO
%token EQTOKEN_DOUBLEBUFFER
%token EQTOKEN_FULLSCREEN
%token EQTOKEN_DECORATION
%token EQTOKEN_PLANES
%token EQTOKEN_COLOR
%token EQTOKEN_ALPHA
%token EQTOKEN_DEPTH
%token EQTOKEN_STENCIL
%token EQTOKEN_ON
%token EQTOKEN_OFF
%token EQTOKEN_AUTO
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
%token EQTOKEN_EYE
%token EQTOKEN_EYE_BASE
%token EQTOKEN_BUFFER
%token EQTOKEN_CLEAR
%token EQTOKEN_DRAW
%token EQTOKEN_READBACK
%token EQTOKEN_CYCLOP
%token EQTOKEN_LEFT
%token EQTOKEN_RIGHT
%token EQTOKEN_VIEWPORT
%token EQTOKEN_RANGE
%token EQTOKEN_DISPLAY
%token EQTOKEN_SCREEN
%token EQTOKEN_WALL
%token EQTOKEN_BOTTOM_LEFT
%token EQTOKEN_BOTTOM_RIGHT
%token EQTOKEN_TOP_LEFT
%token EQTOKEN_SYNC
%token EQTOKEN_LATENCY
%token EQTOKEN_SWAPBARRIER
%token EQTOKEN_OUTPUTFRAME
%token EQTOKEN_INPUTFRAME

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
%type <_int>            INTEGER IATTR;
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
     EQTOKEN_CONNECTION_SATTR_HOSTNAME STRING
     {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, $2 );
     }
     | EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND STRING
     {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_TYPE connectionType 
     { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, $2 ); 
     }
     | EQTOKEN_CONNECTION_IATTR_TCPIP_PORT UNSIGNED
     {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT UNSIGNED
     {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINTS_STEREO IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_STEREO, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DOUBLEBUFFER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_FULLSCREEN, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINTS_DECORATION IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DECORATION, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_COLOR IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ALPHA IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_DEPTH IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_STENCIL IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, $2 );
     }
     | EQTOKEN_CONFIG_FATTR_EYE_BASE FLOAT
     {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, $2 );
     }

connectionType: EQTOKEN_TCPIP { $$ = eqNet::Connection::TYPE_TCPIP; };

server: EQTOKEN_SERVER '{' { server = loader->createServer(); }
        configs '}'

configs: config | configs config
config: EQTOKEN_CONFIG '{' { config = loader->createConfig(); }
        configFields
        nodes compounds '}' { server->addConfig( config ); config = NULL; }
configFields: /*null*/ | configField | configFields configField
configField:
    EQTOKEN_LATENCY UNSIGNED  { config->setLatency( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' configAttributes '}'
configAttributes: /*null*/ | configAttribute | configAttributes configAttribute
configAttribute:
    EQTOKEN_EYE_BASE FLOAT { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, $2 ); }

nodes: node | nodes node
node: appNode | otherNode
otherNode: EQTOKEN_NODE '{' { node = loader->createNode(); }
               connections
               nodeFields
               pipes '}' { config->addNode( node ); node = NULL; }
appNode: EQTOKEN_APPNODE '{' { node = loader->createNode(); }
            connections
            nodeFields
            pipes '}' { config->addApplicationNode( node ); node = NULL; }
nodeFields: /*null*/ | nodeField | nodeFields nodeField
nodeField: /*TODO*/

connections: /*null*/ 
             { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             }
             | connection | connections connection
connection: EQTOKEN_CONNECTION 
            '{' { connectionDescription = new eqs::ConnectionDescription(); }
            connectionFields '}' 
             { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             }
connectionFields: /*null*/ | connectionField | 
                      connectionFields connectionField
connectionField:
    EQTOKEN_TYPE connectionType { connectionDescription->type = $2; }
    | EQTOKEN_HOSTNAME STRING   { connectionDescription->hostname = $2; }
    | EQTOKEN_COMMAND STRING    { connectionDescription->launchCommand = $2; }
    | EQTOKEN_TIMEOUT UNSIGNED  { connectionDescription->launchTimeout = $2; }


pipes: pipe | pipes pipe
pipe: EQTOKEN_PIPE '{' { eqPipe = loader->createPipe(); }
        pipeFields
        windows    '}' { node->addPipe( eqPipe ); eqPipe = NULL; }
pipeFields: /*null*/ | pipeField | pipeFields pipeField
pipeField:
    EQTOKEN_DISPLAY UNSIGNED         { eqPipe->setDisplay( $2 ); }
    | EQTOKEN_SCREEN UNSIGNED        { eqPipe->setScreen( $2 ); }
    | EQTOKEN_VIEWPORT viewport 
        {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)$2[0], (int)$2[1],
                                                      (int)$2[2], (int)$2[3] ));
        }

windows: window | windows window
window: EQTOKEN_WINDOW '{' { window = loader->createWindow(); }
        windowFields
        channels '}' { eqPipe->addWindow( window ); window = NULL; }
windowFields: /*null*/ | windowField | windowFields windowField
windowField: 
    EQTOKEN_ATTRIBUTES '{' 
    windowAttributes '}'
    | EQTOKEN_NAME STRING              { window->setName( $2 ); }
    | EQTOKEN_VIEWPORT viewport
        {
            if( $2[2] > 1 || $2[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)$2[0], 
                                          (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                window->setViewport( eq::Viewport($2[0], $2[1], $2[2], $2[3])); 
        }
windowAttributes: /*null*/ | windowAttribute | windowAttributes windowAttribute
windowAttribute:
    EQTOKEN_HINTS '{' windowHints '}'
    | EQTOKEN_PLANES '{' windowPlanes '}'
windowHints: /*null*/ | windowHint | windowHints windowHint
windowHint:
    EQTOKEN_STEREO IATTR
        { window->setIAttribute( eq::Window::IATTR_HINTS_STEREO, $2 ); }
    | EQTOKEN_DOUBLEBUFFER IATTR
        { window->setIAttribute( eq::Window::IATTR_HINTS_DOUBLEBUFFER, $2 ); }
    | EQTOKEN_FULLSCREEN IATTR
        { window->setIAttribute( eq::Window::IATTR_HINTS_FULLSCREEN, $2 ); }
    | EQTOKEN_DECORATION IATTR
        { window->setIAttribute( eq::Window::IATTR_HINTS_DECORATION, $2 ); }
windowPlanes: /*null*/ | windowPlane | windowPlanes windowPlane
windowPlane:
    EQTOKEN_COLOR IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, $2 ); }
    | EQTOKEN_ALPHA IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, $2 ); }
    | EQTOKEN_DEPTH IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, $2 ); }
    | EQTOKEN_STENCIL IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, $2 ); }
                     
channels: channel | channels channel
channel: EQTOKEN_CHANNEL '{' { channel = loader->createChannel(); }
         channelFields
        '}' { window->addChannel( channel ); channel = NULL; }
channelFields:
     /*null*/ | channelField | channelFields channelField
channelField: 
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
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              }
          compoundFields 
          compoundChildren
          compoundFields
          '}' { eqCompound = eqCompound->getParent(); } 

compoundChildren: /*null*/ | compounds

compoundFields: /*null*/ | compoundField |
                    compoundFields compoundField
compoundField: 
    EQTOKEN_NAME STRING { eqCompound->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
    {
         eqs::Channel* channel = config->findChannel( $2 );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    }
    | EQTOKEN_TASK '['   { eqCompound->setTasks( eqs::Compound::TASK_NONE ); }
        compoundTasks ']'
    | EQTOKEN_EYE  '['   { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED ); }
        compoundEyes  ']'
    | EQTOKEN_BUFFER '[' { eqCompound->setBuffers( eq::Frame::BUFFER_UNDEFINED );}
        compoundBuffers ']'
    | EQTOKEN_VIEWPORT viewport
        { eqCompound->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] )); }
    | EQTOKEN_RANGE '[' FLOAT FLOAT ']'
        { eqCompound->setRange( eq::Range( $3, $4 )); }
    | wall
    | swapBarrier
    | outputFrame
    | inputFrame

compoundTasks: /*null*/ | compoundTask | compoundTasks compoundTask
compoundTask:
    EQTOKEN_CLEAR      { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); }
    | EQTOKEN_DRAW     { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); }
    | EQTOKEN_READBACK { eqCompound->enableTask( eqs::Compound::TASK_READBACK ); }

compoundEyes: /*null*/ | compoundEye | compoundEyes compoundEye
compoundEye:
    EQTOKEN_CYCLOP  { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); }
    | EQTOKEN_LEFT  { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); }
    | EQTOKEN_RIGHT { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); }

compoundBuffers: /*null*/ | compoundBuffer | compoundBuffers compoundBuffer
compoundBuffer:
    EQTOKEN_COLOR    { eqCompound->enableBuffer( eq::Frame::BUFFER_COLOR ); }
    | EQTOKEN_DEPTH  { eqCompound->enableBuffer( eq::Frame::BUFFER_DEPTH ); }

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
        eqCompound->setWall( wall );
    }

swapBarrier: EQTOKEN_SWAPBARRIER '{' { swapBarrier = new eqs::SwapBarrier(); }
    swapBarrierFields '}'
        { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        } 
swapBarrierFields: /*null*/ | swapBarrierField 
    | swapBarrierFields swapBarrierField
swapBarrierField: 
    EQTOKEN_NAME STRING { swapBarrier->setName( $2 ); }

outputFrame : EQTOKEN_OUTPUTFRAME '{' { frame = new eqs::Frame(); }
    frameFields '}'
        { 
            eqCompound->addOutputFrame( frame );
            frame = NULL;
        } 
inputFrame: EQTOKEN_INPUTFRAME '{' { frame = new eqs::Frame(); }
    frameFields '}'
        { 
            eqCompound->addInputFrame( frame );
            frame = NULL;
        } 
frameFields: /*null*/ | frameField | frameFields frameField
frameField: 
    EQTOKEN_NAME STRING { frame->setName( $2 ); }

viewport: '[' FLOAT FLOAT FLOAT FLOAT ']'
     { 
         $$[0] = $2;
         $$[1] = $3;
         $$[2] = $4;
         $$[3] = $5;
     }

IATTR:
    EQTOKEN_ON     { $$ = eq::ON; }
    | EQTOKEN_OFF  { $$ = eq::OFF; }
    | EQTOKEN_AUTO { $$ = eq::AUTO; }
    | INTEGER      { $$ = $1; }

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
