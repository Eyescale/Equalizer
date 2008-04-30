
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
        static eqs::Loader* loader = 0;
        static std::string  stringBuf;
        
        static eqs::Server*      server = 0;
        static eqs::Config*      config = 0;
        static eqs::Node*        node = 0;
        static eqs::Pipe*        eqPipe = 0; // avoid name clash with pipe()
        static eqs::Window*      window = 0;
        static eqs::Channel*     channel = 0;
        static eqs::Compound*    eqCompound = 0; // avoid name clash on Darwin
        static eqs::SwapBarrier* swapBarrier = 0;
        static eqs::Frame*       frame = 0;
        static eqBase::RefPtr<eqNet::ConnectionDescription> 
            connectionDescription;
        static eqs::Wall         wall;
        static eqs::Projection   projection;
        static uint32_t          flags = 0;
    }

    using namespace std;
    using namespace eqLoader;
    using namespace eqBase;

    int eqLoader_lex();

    #define yylineno eqLoader_lineno
    void yyerror( char *errmsg );
    extern char* yytext;
    extern FILE*       yyin;
    extern const char* yyinString;
    extern int yylineno;
%}

%token EQTOKEN_GLOBAL
%token EQTOKEN_CONNECTION_SATTR_HOSTNAME
%token EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND
%token EQTOKEN_CONNECTION_CATTR_LAUNCH_COMMAND_QUOTE
%token EQTOKEN_CONNECTION_IATTR_TYPE
%token EQTOKEN_CONNECTION_IATTR_TCPIP_PORT
%token EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT
%token EQTOKEN_CONFIG_FATTR_EYE_BASE
%token EQTOKEN_PIPE_IATTR_HINT_THREAD
%token EQTOKEN_WINDOW_IATTR_HINT_STEREO
%token EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER
%token EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN
%token EQTOKEN_WINDOW_IATTR_HINT_DECORATION
%token EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC
%token EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE
%token EQTOKEN_WINDOW_IATTR_HINT_STATISTICS
%token EQTOKEN_WINDOW_IATTR_PLANES_COLOR
%token EQTOKEN_WINDOW_IATTR_PLANES_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_DEPTH
%token EQTOKEN_WINDOW_IATTR_PLANES_STENCIL
%token EQTOKEN_COMPOUND_IATTR_STEREO_MODE
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_COMPOUND_IATTR_UPDATE_FOV
%token EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS
%token EQTOKEN_SERVER
%token EQTOKEN_CONFIG
%token EQTOKEN_APPNODE
%token EQTOKEN_NODE
%token EQTOKEN_PIPE
%token EQTOKEN_WINDOW
%token EQTOKEN_ATTRIBUTES
%token EQTOKEN_HINT_STEREO
%token EQTOKEN_HINT_DOUBLEBUFFER
%token EQTOKEN_HINT_FULLSCREEN
%token EQTOKEN_HINT_DECORATION
%token EQTOKEN_HINT_STATISTICS
%token EQTOKEN_HINT_SWAPSYNC
%token EQTOKEN_HINT_DRAWABLE
%token EQTOKEN_HINT_THREAD
%token EQTOKEN_PLANES_COLOR
%token EQTOKEN_PLANES_ALPHA
%token EQTOKEN_PLANES_DEPTH
%token EQTOKEN_PLANES_STENCIL
%token EQTOKEN_ON
%token EQTOKEN_OFF
%token EQTOKEN_AUTO
%token EQTOKEN_FASTEST
%token EQTOKEN_NICEST
%token EQTOKEN_QUAD
%token EQTOKEN_ANAGLYPH
%token EQTOKEN_RED
%token EQTOKEN_GREEN
%token EQTOKEN_BLUE
%token EQTOKEN_HORIZONTAL
%token EQTOKEN_VERTICAL
%token EQTOKEN_CHANNEL
%token EQTOKEN_COMPOUND
%token EQTOKEN_CONNECTION
%token EQTOKEN_NAME
%token EQTOKEN_TYPE
%token EQTOKEN_TCPIP
%token EQTOKEN_SDP
%token EQTOKEN_HOSTNAME
%token EQTOKEN_COMMAND
%token EQTOKEN_COMMAND_QUOTE
%token EQTOKEN_TIMEOUT
%token EQTOKEN_TCPIP_PORT
%token EQTOKEN_TASK
%token EQTOKEN_EYE
%token EQTOKEN_EYE_BASE
%token EQTOKEN_BUFFER
%token EQTOKEN_CLEAR
%token EQTOKEN_DRAW
%token EQTOKEN_ASSEMBLE
%token EQTOKEN_READBACK
%token EQTOKEN_COLOR
%token EQTOKEN_DEPTH
%token EQTOKEN_CYCLOP
%token EQTOKEN_LEFT
%token EQTOKEN_RIGHT
%token EQTOKEN_VIEWPORT
%token EQTOKEN_RANGE
%token EQTOKEN_PERIOD
%token EQTOKEN_PHASE
%token EQTOKEN_PIXEL
%token EQTOKEN_PORT
%token EQTOKEN_DEVICE
%token EQTOKEN_WALL
%token EQTOKEN_BOTTOM_LEFT
%token EQTOKEN_BOTTOM_RIGHT
%token EQTOKEN_TOP_LEFT
%token EQTOKEN_PROJECTION
%token EQTOKEN_ORIGIN
%token EQTOKEN_DISTANCE
%token EQTOKEN_FOV
%token EQTOKEN_HPR
%token EQTOKEN_SYNC
%token EQTOKEN_LATENCY
%token EQTOKEN_SWAPBARRIER
%token EQTOKEN_OUTPUTFRAME
%token EQTOKEN_INPUTFRAME
%token EQTOKEN_STEREO_MODE
%token EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_UPDATE_FOV
%token EQTOKEN_PBUFFER

%token EQTOKEN_STRING
%token EQTOKEN_CHARACTER
%token EQTOKEN_FLOAT
%token EQTOKEN_INTEGER
%token EQTOKEN_UNSIGNED

%union{
    const char*             _string;
    char                    _character;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::ConnectionType   _connectionType;
    float                   _viewport[4];
}

%type <_string>         STRING;
%type <_character>      CHARACTER;
%type <_int>            INTEGER IATTR;
%type <_unsigned>       UNSIGNED colorMask colorMaskBit colorMaskBits;
%type <_connectionType> connectionType;
%type <_viewport>       viewport;
%type <_float>          FLOAT;

%%

file:   global server | global config;

global: EQTOKEN_GLOBAL '{' globals '}' 
        | /* null */
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
     | EQTOKEN_CONNECTION_CATTR_LAUNCH_COMMAND_QUOTE CHARACTER
     {
         eqs::Global::instance()->setConnectionCAttribute(
             eqs::ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE, $2 );
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
     | EQTOKEN_CONFIG_FATTR_EYE_BASE FLOAT
     {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, $2 );
     }
     | EQTOKEN_PIPE_IATTR_HINT_THREAD IATTR
     {
         eqs::Global::instance()->setPipeIAttribute(
             eqs::Pipe::IATTR_HINT_THREAD, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STEREO IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STEREO, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DOUBLEBUFFER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_FULLSCREEN, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DECORATION IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DECORATION, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_SWAPSYNC, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DRAWABLE, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STATISTICS IATTR
     {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STATISTICS, $2 );
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
     | EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS IATTR
     {
         eqs::Global::instance()->setChannelIAttribute(
             eq::Channel::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_MODE IATTR 
     { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_MODE, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK colorMask 
     { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK colorMask 
     { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_UPDATE_FOV IATTR
     {
         eqs::Global::instance()->setCompoundIAttribute(
             eqs::Compound::IATTR_UPDATE_FOV, $2 );
     }

connectionType: 
    EQTOKEN_TCPIP { $$ = eqNet::CONNECTIONTYPE_TCPIP; }
    | EQTOKEN_SDP { $$ = eqNet::CONNECTIONTYPE_SDP; }

server: EQTOKEN_SERVER '{' { server = loader->createServer(); }
        serverConnections
        configs '}'

serverConnections: /*null*/ 
             | serverConnection | serverConnections serverConnection
serverConnection: EQTOKEN_CONNECTION 
        '{' { 
                connectionDescription = new eqs::ConnectionDescription;
                connectionDescription->setHostname( "" );
                connectionDescription->TCPIP.port = EQ_DEFAULT_PORT;
            }
            connectionFields '}' 
            { 
                server->addConnectionDescription( connectionDescription );
                connectionDescription = 0;
            }

configs: config | configs config
config: EQTOKEN_CONFIG '{' { config = loader->createConfig(); }
        configFields
        '}' {
                if( server ) 
                {
                    server->addConfig( config ); 
                    config = 0;
                } // else leave config for Loader::parseConfig()
        }
configFields: /*null*/ | configField | configFields configField
configField:
    nodes
    | compounds
    | EQTOKEN_LATENCY UNSIGNED  { config->setLatency( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' configAttributes '}'
configAttributes: /*null*/ | configAttribute | configAttributes configAttribute
configAttribute:
    EQTOKEN_EYE_BASE FLOAT { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, $2 ); }

nodes: node | nodes node
node: appNode | renderNode
renderNode: EQTOKEN_NODE '{' { node = loader->createNode(); }
               nodeFields
               '}' { 
                        if( node->getConnectionDescriptions().empty( ))
                            node->addConnectionDescription(
                                new eqs::ConnectionDescription );

                        config->addNode( node );
                        node = 0; 
                   }
appNode: EQTOKEN_APPNODE '{' { node = loader->createNode(); }
            nodeFields
            '}' { config->addApplicationNode( node ); node = 0; }
nodeFields: /*null*/ | nodeField | nodeFields nodeField
nodeField: EQTOKEN_NAME STRING            { node->setName( $2 ); }
    | connections
    | pipes                
connections: /*null*/ | connection | connections connection
connection: EQTOKEN_CONNECTION 
            '{' { connectionDescription = new eqs::ConnectionDescription; }
            connectionFields '}' 
             { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             }
connectionFields: /*null*/ | connectionField | 
                      connectionFields connectionField
connectionField:
    EQTOKEN_TYPE connectionType   { connectionDescription->type = $2; }
    | EQTOKEN_HOSTNAME  STRING    { connectionDescription->setHostname($2); }
    | EQTOKEN_COMMAND   STRING  { connectionDescription->setLaunchCommand($2); }
    | EQTOKEN_COMMAND_QUOTE CHARACTER { connectionDescription->launchCommandQuote = $2; }
    | EQTOKEN_TIMEOUT   UNSIGNED  { connectionDescription->launchTimeout = $2; }
    | EQTOKEN_TCPIP_PORT UNSIGNED { connectionDescription->TCPIP.port = $2; }


pipes: pipe | pipes pipe
pipe: EQTOKEN_PIPE '{' { eqPipe = loader->createPipe(); }
        pipeFields
        '}' { node->addPipe( eqPipe ); eqPipe = 0; }
pipeFields: /*null*/ | pipeField | pipeFields pipeField
pipeField:
    windows   
    | EQTOKEN_ATTRIBUTES '{' pipeAttributes '}'
    | EQTOKEN_NAME     STRING          { eqPipe->setName( $2 ); }
    | EQTOKEN_PORT     UNSIGNED        { eqPipe->setPort( $2 ); }
    | EQTOKEN_DEVICE   UNSIGNED        { eqPipe->setDevice( $2 ); }
    | EQTOKEN_VIEWPORT viewport 
        {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)$2[0], (int)$2[1],
                                                      (int)$2[2], (int)$2[3] ));
        }
pipeAttributes: /*null*/ | pipeAttribute | pipeAttributes pipeAttribute
pipeAttribute:
    EQTOKEN_HINT_THREAD IATTR
        { eqPipe->setIAttribute( eqs::Pipe::IATTR_HINT_THREAD, $2 ); }

windows: window | windows window
window: EQTOKEN_WINDOW '{' { window = loader->createWindow(); }
        windowFields
        '}' { eqPipe->addWindow( window ); window = 0; }
windowFields: /*null*/ | windowField | windowFields windowField
windowField: 
    channels
    | EQTOKEN_ATTRIBUTES '{' windowAttributes '}'
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
    EQTOKEN_HINT_STEREO IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_STEREO, $2 ); }
    | EQTOKEN_HINT_DOUBLEBUFFER IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER, $2 ); }
    | EQTOKEN_HINT_FULLSCREEN IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_FULLSCREEN, $2 ); }
    | EQTOKEN_HINT_DECORATION IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_DECORATION, $2 ); }
    | EQTOKEN_HINT_SWAPSYNC IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_SWAPSYNC, $2 ); }
    | EQTOKEN_HINT_DRAWABLE IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_DRAWABLE, $2 ); }
    | EQTOKEN_HINT_STATISTICS IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_STATISTICS, $2 ); }
    | EQTOKEN_PLANES_COLOR IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, $2 ); }
    | EQTOKEN_PLANES_ALPHA IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, $2 ); }
    | EQTOKEN_PLANES_DEPTH IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, $2 ); }
    | EQTOKEN_PLANES_STENCIL IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, $2 ); }
                     
channels: channel | channels channel
channel: EQTOKEN_CHANNEL '{' { channel = loader->createChannel(); }
         channelFields
        '}' { window->addChannel( channel ); channel = 0; }
channelFields:
     /*null*/ | channelField | channelFields channelField
channelField: 
    EQTOKEN_NAME STRING { channel->setName( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' 
    channelAttributes '}'
    | EQTOKEN_VIEWPORT viewport
        {
            if( $2[2] > 1 || $2[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)$2[0],
                                          (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                channel->setViewport(eq::Viewport( $2[0], $2[1], $2[2], $2[3]));
        }
channelAttributes: /*null*/ | channelAttribute | channelAttributes channelAttribute
channelAttribute:
    EQTOKEN_HINT_STATISTICS IATTR
        { channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, $2 ); }


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
          '}' { eqCompound = eqCompound->getParent(); } 

compoundFields: /*null*/ | compoundField |
                    compoundFields compoundField
compoundField: 
    compound
    | EQTOKEN_NAME STRING { eqCompound->setName( $2 ); }
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
    | EQTOKEN_EYE  '['   { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED );}
        compoundEyes  ']'
    | EQTOKEN_BUFFER '[' { flags = eq::Frame::BUFFER_NONE; }
        buffers ']' { eqCompound->setBuffers( flags ); flags = 0; }
    | EQTOKEN_VIEWPORT viewport
        { eqCompound->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] ));}
    | EQTOKEN_RANGE '[' FLOAT FLOAT ']'
        { eqCompound->setRange( eq::Range( $3, $4 )); }
    | EQTOKEN_PERIOD UNSIGNED { eqCompound->setPeriod( $2 ); }
    | EQTOKEN_PHASE  UNSIGNED { eqCompound->setPhase( $2 ); }
    | EQTOKEN_PIXEL '[' UNSIGNED UNSIGNED ']'
        { eqCompound->setPixel( eq::Pixel( $3, $4 )); }
    | wall
    | projection
    | swapBarrier
    | outputFrame
    | inputFrame
    | EQTOKEN_ATTRIBUTES '{' compoundAttributes '}'

compoundTasks: /*null*/ | compoundTask | compoundTasks compoundTask
compoundTask:
    EQTOKEN_CLEAR      { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); }
    | EQTOKEN_DRAW     { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); }
    | EQTOKEN_ASSEMBLE { eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);}
    | EQTOKEN_READBACK { eqCompound->enableTask( eqs::Compound::TASK_READBACK);}

compoundEyes: /*null*/ | compoundEye | compoundEyes compoundEye
compoundEye:
    EQTOKEN_CYCLOP  { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP_BIT ); }
    | EQTOKEN_LEFT  { eqCompound->enableEye( eqs::Compound::EYE_LEFT_BIT ); }
    | EQTOKEN_RIGHT { eqCompound->enableEye( eqs::Compound::EYE_RIGHT_BIT ); }

buffers: /*null*/ | buffer | buffers buffer
buffer:
    EQTOKEN_COLOR    { flags |= eq::Frame::BUFFER_COLOR; }
    | EQTOKEN_DEPTH  { flags |= eq::Frame::BUFFER_DEPTH; }

wall: EQTOKEN_WALL '{' { wall = eqs::Wall(); } 
    wallFields '}' { eqCompound->setWall( wall ); }

wallFields:  /*null*/ | wallField | wallFields wallField
wallField:
    EQTOKEN_BOTTOM_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomLeft = vmml::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_BOTTOM_RIGHT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomRight = vmml::Vector3f( $3, $4, $5 ); }
   |  EQTOKEN_TOP_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.topLeft = vmml::Vector3f( $3, $4, $5 ); }

projection: EQTOKEN_PROJECTION '{' { projection = eqs::Projection(); } 
    projectionFields '}' { eqCompound->setProjection( projection ); }

projectionFields:  /*null*/ | projectionField | projectionFields projectionField
projectionField:
    EQTOKEN_ORIGIN  '[' FLOAT FLOAT FLOAT ']'
        { projection.origin = vmml::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_DISTANCE FLOAT
        { projection.distance = $2; }
    | EQTOKEN_FOV  '[' FLOAT FLOAT ']'
        { projection.fov = vmml::Vector2f( $3, $4 ); }
    | EQTOKEN_HPR  '[' FLOAT FLOAT FLOAT ']'
        { projection.hpr = vmml::Vector3f( $3, $4, $5 ); }

swapBarrier: EQTOKEN_SWAPBARRIER '{' { swapBarrier = new eqs::SwapBarrier; }
    swapBarrierFields '}'
        { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        } 
swapBarrierFields: /*null*/ | swapBarrierField 
    | swapBarrierFields swapBarrierField
swapBarrierField: 
    EQTOKEN_NAME STRING { swapBarrier->setName( $2 ); }

outputFrame : EQTOKEN_OUTPUTFRAME '{' { frame = new eqs::Frame; }
    frameFields '}'
        { 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        } 
inputFrame: EQTOKEN_INPUTFRAME '{' { frame = new eqs::Frame; }
    frameFields '}'
        { 
            eqCompound->addInputFrame( frame );
            frame = 0;
        } 
frameFields: /*null*/ | frameField | frameFields frameField
frameField: 
    EQTOKEN_NAME STRING { frame->setName( $2 ); }
    | EQTOKEN_VIEWPORT viewport
        { frame->setViewport(eq::Viewport( $2[0], $2[1], $2[2], $2[3])); }
    | EQTOKEN_BUFFER '[' { flags = eq::Frame::BUFFER_NONE; }
        buffers ']' { frame->setBuffers( flags ); flags = 0; }

compoundAttributes: /*null*/ | compoundAttribute | compoundAttributes compoundAttribute
compoundAttribute:
    EQTOKEN_STEREO_MODE IATTR 
        { eqCompound->setIAttribute( eqs::Compound::IATTR_STEREO_MODE, $2 ); }
    | EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK colorMask
        { eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, $2 ); }
    | EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK colorMask
        { eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, $2 ); }
    | EQTOKEN_UPDATE_FOV IATTR
        { eqCompound->setIAttribute(eqs::Compound::IATTR_UPDATE_FOV, $2 ); }

viewport: '[' FLOAT FLOAT FLOAT FLOAT ']'
     { 
         $$[0] = $2;
         $$[1] = $3;
         $$[2] = $4;
         $$[3] = $5;
     }

colorMask: '[' colorMaskBits ']' { $$ = $2; }
colorMaskBits: 
    /*null*/ { $$ =eqs::Compound::COLOR_MASK_NONE; }
    | colorMaskBit { $$ = $1; }
    | colorMaskBits colorMaskBit { $$ = ($1 | $2);}
colorMaskBit:
    EQTOKEN_RED     { $$ = eqs::Compound::COLOR_MASK_RED; }
    | EQTOKEN_GREEN { $$ = eqs::Compound::COLOR_MASK_GREEN; }
    | EQTOKEN_BLUE  { $$ = eqs::Compound::COLOR_MASK_BLUE; }

IATTR:
    EQTOKEN_ON           { $$ = eq::ON; }
    | EQTOKEN_OFF        { $$ = eq::OFF; }
    | EQTOKEN_AUTO       { $$ = eq::AUTO; }
    | EQTOKEN_FASTEST    { $$ = eq::FASTEST; }
    | EQTOKEN_HORIZONTAL { $$ = eq::HORIZONTAL; }
    | EQTOKEN_NICEST     { $$ = eq::NICEST; }
    | EQTOKEN_QUAD       { $$ = eq::QUAD; }
    | EQTOKEN_ANAGLYPH   { $$ = eq::ANAGLYPH; } 
    | EQTOKEN_VERTICAL   { $$ = eq::VERTICAL; }
    | EQTOKEN_WINDOW     { $$ = eq::WINDOW; }
    | EQTOKEN_PBUFFER    { $$ = eq::PBUFFER; }
    | INTEGER            { $$ = $1; }

STRING: EQTOKEN_STRING
     {
         stringBuf = yytext;
         stringBuf.erase( 0, 1 );                  // Leading '"'
         stringBuf.erase( stringBuf.size()-1, 1 ); // Trailing '"'
         $$ = stringBuf.c_str(); 
     }

CHARACTER: EQTOKEN_CHARACTER               { $$ = yytext[1]; }

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
eqs::Server* eqs::Loader::loadFile( const string& filename )
{
    EQASSERTINFO( !loader, "Config file loader is not reentrant" );
    loader = this;

    yyin       = fopen( filename.c_str(), "r" );
    yyinString = 0;

    if( !yyin )
    {
        EQERROR << "Can't open config file " << filename << endl;
        loader = 0;
        return 0;
    }

    server = 0;
    config = 0;
    eqLoader_parse();

    fclose( yyin );
    loader = 0;
    return server;
}

void eqs::Loader::_parseString( const char* data )
{
    EQASSERTINFO( !loader, "Config file loader is not reentrant" );
    loader = this;

    yyin       = 0;
    yyinString = data;

    server = 0;
    config = 0;
    eqLoader_parse();

    loader = 0;
}

eqs::Config* eqs::Loader::parseConfig( const char* data )
{
    _parseString( data );
    return config;
}

eqs::Server* eqs::Loader::parseServer( const char* data )
{
    _parseString( data );
    return server;
}
