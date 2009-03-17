
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

%{
#include "loader.h"

#include "canvas.h"
#include "compound.h"
#include "frame.h"
#include "global.h"
#include "layout.h"
#include "loadBalancer.h"
#include "node.h"
#include "paths.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"
#include "swapBarrier.h"
#include "view.h"
#include "window.h"

#include <eq/base/base.h>
#include <string>

    namespace eq
    {
    namespace loader
    {
        static eq::server::Loader*      loader = 0;
        
        static eq::server::ServerPtr    server;
        static eq::server::Config*      config = 0;
        static eq::server::Node*        node = 0;
        static eq::server::Pipe*        eqPipe = 0; // avoid name clash
        static eq::server::Window*      window = 0;
        static eq::server::Channel*     channel = 0;
        static eq::server::Layout*      layout = 0;
        static eq::server::View*        view = 0;
        static eq::server::Canvas*      canvas = 0;
        static eq::server::Segment*     segment = 0;
        static eq::server::Compound*    eqCompound = 0; // avoid name clash
        static eq::server::LoadBalancer* loadBalancer = 0;
        static eq::server::SwapBarrier* swapBarrier = 0;
        static eq::server::Frame*       frame = 0;
        static eq::server::ConnectionDescriptionPtr connectionDescription;
        static eq::Wall           wall;
        static eq::Projection     projection;
        static uint32_t           flags = 0;
    }
    }

    using namespace eq::base;
    using namespace eq::loader;

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
%token EQTOKEN_CONNECTION_IATTR_PORT
%token EQTOKEN_CONNECTION_IATTR_BANDWIDTH
%token EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT
%token EQTOKEN_CONFIG_FATTR_EYE_BASE
%token EQTOKEN_NODE_IATTR_THREAD_MODEL
%token EQTOKEN_NODE_IATTR_HINT_STATISTICS
%token EQTOKEN_PIPE_IATTR_HINT_THREAD
%token EQTOKEN_WINDOW_IATTR_HINT_STEREO
%token EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER
%token EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN
%token EQTOKEN_WINDOW_IATTR_HINT_DECORATION
%token EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC
%token EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE
%token EQTOKEN_WINDOW_IATTR_HINT_STATISTICS
%token EQTOKEN_WINDOW_IATTR_HINT_SCREENSAVER
%token EQTOKEN_WINDOW_IATTR_PLANES_COLOR
%token EQTOKEN_WINDOW_IATTR_PLANES_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_DEPTH
%token EQTOKEN_WINDOW_IATTR_PLANES_STENCIL
%token EQTOKEN_WINDOW_IATTR_PLANES_ACCUM
%token EQTOKEN_WINDOW_IATTR_PLANES_ACCUM_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_SAMPLES
%token EQTOKEN_COMPOUND_IATTR_STEREO_MODE
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_COMPOUND_IATTR_UPDATE_FOV
%token EQTOKEN_COMPOUND_IATTR_HINT_OFFSET
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
%token EQTOKEN_HINT_SCREENSAVER
%token EQTOKEN_PLANES_COLOR
%token EQTOKEN_PLANES_ALPHA
%token EQTOKEN_PLANES_DEPTH
%token EQTOKEN_PLANES_STENCIL
%token EQTOKEN_PLANES_ACCUM
%token EQTOKEN_PLANES_ACCUM_ALPHA
%token EQTOKEN_PLANES_SAMPLES
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
%token EQTOKEN_DFR
%token EQTOKEN_DDS
%token EQTOKEN_FRAMERATE
%token EQTOKEN_DPLEX
%token EQTOKEN_CHANNEL
%token EQTOKEN_LAYOUT
%token EQTOKEN_VIEW
%token EQTOKEN_CANVAS
%token EQTOKEN_SEGMENT
%token EQTOKEN_COMPOUND
%token EQTOKEN_LOADBALANCER
%token EQTOKEN_DAMPING
%token EQTOKEN_CONNECTION
%token EQTOKEN_NAME
%token EQTOKEN_TYPE
%token EQTOKEN_TCPIP
%token EQTOKEN_SDP
%token EQTOKEN_TEXTURE
%token EQTOKEN_MEMORY
%token EQTOKEN_HOSTNAME
%token EQTOKEN_COMMAND
%token EQTOKEN_COMMAND_QUOTE
%token EQTOKEN_TIMEOUT
%token EQTOKEN_TCPIP_PORT
%token EQTOKEN_TASK
%token EQTOKEN_EYE
%token EQTOKEN_EYE_BASE
%token EQTOKEN_THREAD_MODEL
%token EQTOKEN_ASYNC
%token EQTOKEN_DRAW_SYNC
%token EQTOKEN_LOCAL_SYNC
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
%token EQTOKEN_DRAWABLE
%token EQTOKEN_FBO_COLOR
%token EQTOKEN_FBO_DEPTH
%token EQTOKEN_FBO_STENCIL
%token EQTOKEN_RANGE
%token EQTOKEN_PERIOD
%token EQTOKEN_PHASE
%token EQTOKEN_PIXEL
%token EQTOKEN_PORT
%token EQTOKEN_BANDWIDTH
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
%token EQTOKEN_LATENCY
%token EQTOKEN_SWAPBARRIER
%token EQTOKEN_NVGROUP 
%token EQTOKEN_NVBARRIER
%token EQTOKEN_OUTPUTFRAME
%token EQTOKEN_INPUTFRAME
%token EQTOKEN_STEREO_MODE
%token EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_UPDATE_FOV
%token EQTOKEN_HINT_OFFSET
%token EQTOKEN_PBUFFER
%token EQTOKEN_FBO
%token EQTOKEN_MODE
%token EQTOKEN_2D
%token EQTOKEN_DB
%token EQTOKEN_ZOOM
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
    eq::net::ConnectionType   _connectionType;
    eq::server::LoadBalancer::Mode _loadBalancerMode;
    float                   _viewport[4];
}

%type <_string>           STRING;
%type <_character>        CHARACTER;
%type <_int>              INTEGER IATTR;
%type <_unsigned>         UNSIGNED colorMask colorMaskBit colorMaskBits;
%type <_connectionType>   connectionType;
%type <_loadBalancerMode> loadBalancerMode;
%type <_viewport>         viewport;
%type <_float>            FLOAT;

%%

file:   global server | global config;

global: EQTOKEN_GLOBAL '{' globals '}' 
        | /* null */
        ;

globals: /* null */ | globals global;

global:
     EQTOKEN_CONNECTION_SATTR_HOSTNAME STRING
     {
         eq::server::Global::instance()->setConnectionSAttribute(
             eq::server::ConnectionDescription::SATTR_HOSTNAME, $2 );
     }
     | EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND STRING
     {
         eq::server::Global::instance()->setConnectionSAttribute(
             eq::server::ConnectionDescription::SATTR_LAUNCH_COMMAND, $2 );
     }
     | EQTOKEN_CONNECTION_CATTR_LAUNCH_COMMAND_QUOTE CHARACTER
     {
         eq::server::Global::instance()->setConnectionCAttribute(
             eq::server::ConnectionDescription::CATTR_LAUNCH_COMMAND_QUOTE, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_TYPE connectionType 
     { 
         eq::server::Global::instance()->setConnectionIAttribute( 
             eq::server::ConnectionDescription::IATTR_TYPE, $2 ); 
     }
     | EQTOKEN_CONNECTION_IATTR_TCPIP_PORT UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_TCPIP_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_PORT UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_TCPIP_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_BANDWIDTH UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_BANDWIDTH, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, $2 );
     }
     | EQTOKEN_CONFIG_FATTR_EYE_BASE FLOAT
     {
         eq::server::Global::instance()->setConfigFAttribute(
             eq::server::Config::FATTR_EYE_BASE, $2 );
     }
     | EQTOKEN_NODE_IATTR_THREAD_MODEL IATTR
     {
         eq::server::Global::instance()->setNodeIAttribute(
             eq::Node::IATTR_THREAD_MODEL, $2 );
     }
     | EQTOKEN_NODE_IATTR_HINT_STATISTICS IATTR
     {
         eq::server::Global::instance()->setNodeIAttribute(
             eq::Node::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_PIPE_IATTR_HINT_THREAD IATTR
     {
         eq::server::Global::instance()->setPipeIAttribute(
             eq::server::Pipe::IATTR_HINT_THREAD, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STEREO IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STEREO, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DOUBLEBUFFER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_FULLSCREEN, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DECORATION IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DECORATION, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_SWAPSYNC, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DRAWABLE, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STATISTICS IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_SCREENSAVER IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_SCREENSAVER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_COLOR IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ALPHA IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_DEPTH IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_STENCIL IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ACCUM IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ACCUM, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ACCUM_ALPHA IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ACCUM_ALPHA, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_SAMPLES IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_SAMPLES, $2 );
     }
     | EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS IATTR
     {
         eq::server::Global::instance()->setChannelIAttribute(
             eq::Channel::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_MODE IATTR 
     { 
         eq::server::Global::instance()->setCompoundIAttribute( 
             eq::server::Compound::IATTR_STEREO_MODE, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK colorMask 
     { 
         eq::server::Global::instance()->setCompoundIAttribute( 
             eq::server::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK colorMask 
     { 
         eq::server::Global::instance()->setCompoundIAttribute( 
             eq::server::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, $2 ); 
     }
     | EQTOKEN_COMPOUND_IATTR_UPDATE_FOV IATTR
     {
         EQWARN << "ignoring removed attribute EQ_COMPOUND_IATTR_UPDATE_FOV"
                << std::endl;
     }
     | EQTOKEN_COMPOUND_IATTR_HINT_OFFSET IATTR
     {
         eq::server::Global::instance()->setCompoundIAttribute(
             eq::server::Compound::IATTR_HINT_OFFSET, $2 );
     }

connectionType: 
    EQTOKEN_TCPIP { $$ = eq::net::CONNECTIONTYPE_TCPIP; }
    | EQTOKEN_SDP { $$ = eq::net::CONNECTIONTYPE_SDP; }

server: EQTOKEN_SERVER '{' { server = new eq::server::Server(); }
        serverConnections
        configs '}'

serverConnections: /*null*/ | serverConnections serverConnection
serverConnection: EQTOKEN_CONNECTION 
        '{' { 
                connectionDescription = new eq::server::ConnectionDescription;
                connectionDescription->setHostname( "" );
                connectionDescription->TCPIP.port = EQ_DEFAULT_PORT;
            }
            connectionFields '}' 
            { 
                server->addConnectionDescription( connectionDescription.get( ));
                connectionDescription = 0;
            }

configs: config | configs config
config: EQTOKEN_CONFIG '{'
            {
                config = new eq::server::Config();
                if( server.isValid( )) 
                    server->addConfig( config ); 
            }
        configFields
        '}' {
                if( server.isValid( )) 
                    config = 0;
                // else leave config for Loader::parseConfig()
            }
configFields: /*null*/ | configFields configField
configField:
    node
    | EQTOKEN_NAME STRING       { config->setName( $2 ); }
    | layout
    | canvas
    | compound
    | EQTOKEN_LATENCY UNSIGNED  { config->setLatency( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' configAttributes '}'
configAttributes: /*null*/ | configAttributes configAttribute
configAttribute:
    EQTOKEN_EYE_BASE FLOAT { config->setFAttribute( 
                             eq::server::Config::FATTR_EYE_BASE, $2 ); }

node: appNode | renderNode
renderNode: EQTOKEN_NODE '{' {
                                 node = new eq::server::Node();
                                 config->addNode( node );
                             }
               nodeFields
               '}' { 
                        if( node->getConnectionDescriptions().empty( ))
                            node->addConnectionDescription(
                                new eq::server::ConnectionDescription );
                        node = 0; 
                   }
appNode: EQTOKEN_APPNODE '{' 
            {
                node = new eq::server::Node();
                config->addApplicationNode( node );
            }
            nodeFields
            '}' { node = 0; }
nodeFields: /*null*/ | nodeFields nodeField
nodeField: 
    EQTOKEN_NAME STRING            { node->setName( $2 ); }
    | connection
    | pipe
    | EQTOKEN_ATTRIBUTES '{' nodeAttributes '}'
connection:
    EQTOKEN_CONNECTION 
        '{' { connectionDescription = new eq::server::ConnectionDescription; }
             connectionFields
        '}'  { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             }
connectionFields: /*null*/ | connectionFields connectionField
connectionField:
    EQTOKEN_TYPE connectionType   { connectionDescription->type = $2; }
    | EQTOKEN_HOSTNAME  STRING    { connectionDescription->setHostname($2); }
    | EQTOKEN_COMMAND   STRING  { connectionDescription->setLaunchCommand($2); }
    | EQTOKEN_COMMAND_QUOTE CHARACTER { connectionDescription->launchCommandQuote = $2; }
    | EQTOKEN_TIMEOUT   UNSIGNED  { connectionDescription->launchTimeout = $2; }
    | EQTOKEN_TCPIP_PORT UNSIGNED { connectionDescription->TCPIP.port = $2; }
    | EQTOKEN_PORT UNSIGNED       { connectionDescription->TCPIP.port = $2; }
    | EQTOKEN_BANDWIDTH UNSIGNED  { connectionDescription->bandwidth = $2; }

nodeAttributes: /*null*/ | nodeAttributes nodeAttribute
nodeAttribute:
    EQTOKEN_THREAD_MODEL IATTR 
        { node->setIAttribute( eq::Node::IATTR_THREAD_MODEL, $2 ); }
    | EQTOKEN_HINT_STATISTICS IATTR
        { node->setIAttribute( eq::Node::IATTR_HINT_STATISTICS, $2 ); }


pipe: EQTOKEN_PIPE '{' 
            {
                eqPipe = new eq::server::Pipe();
                node->addPipe( eqPipe );
            }
        pipeFields
        '}' { eqPipe = 0; }
pipeFields: /*null*/ | pipeFields pipeField
pipeField:
    window
    | EQTOKEN_ATTRIBUTES '{' pipeAttributes '}'
    | EQTOKEN_NAME     STRING          { eqPipe->setName( $2 ); }
    | EQTOKEN_PORT     UNSIGNED        { eqPipe->setPort( $2 ); }
    | EQTOKEN_DEVICE   UNSIGNED        { eqPipe->setDevice( $2 ); }
    | EQTOKEN_VIEWPORT viewport 
        {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)$2[0], (int)$2[1],
                                                      (int)$2[2], (int)$2[3] ));
        }
pipeAttributes: /*null*/ | pipeAttributes pipeAttribute
pipeAttribute:
    EQTOKEN_HINT_THREAD IATTR
        { eqPipe->setIAttribute( eq::server::Pipe::IATTR_HINT_THREAD, $2 ); }

window: EQTOKEN_WINDOW '{' 
            {
                window = new eq::server::Window();
                eqPipe->addWindow( window ); 
            }
        windowFields
        '}' { window = 0; }
windowFields: /*null*/ | windowFields windowField
windowField: 
    channel
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
windowAttributes: /*null*/ | windowAttributes windowAttribute
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
    | EQTOKEN_HINT_SCREENSAVER IATTR
        { window->setIAttribute( eq::Window::IATTR_HINT_SCREENSAVER, $2 ); }
    | EQTOKEN_PLANES_COLOR IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, $2 ); }
    | EQTOKEN_PLANES_ALPHA IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, $2 ); }
    | EQTOKEN_PLANES_DEPTH IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, $2 ); }
    | EQTOKEN_PLANES_STENCIL IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, $2 ); }
    | EQTOKEN_PLANES_ACCUM IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_ACCUM, $2 ); }
    | EQTOKEN_PLANES_ACCUM_ALPHA IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_ACCUM_ALPHA, $2 ); }
    | EQTOKEN_PLANES_SAMPLES IATTR
        { window->setIAttribute( eq::Window::IATTR_PLANES_SAMPLES, $2 ); }
                     
channel: EQTOKEN_CHANNEL '{' 
            {
                channel = new eq::server::Channel();
                window->addChannel( channel );
            }
         channelFields
        '}' { channel = 0; }
channelFields: /*null*/ | channelFields channelField
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
    | EQTOKEN_DRAWABLE '[' { flags = eq::Channel::FBO_NONE; }
         drawables ']' { channel->setDrawable( flags ); flags = 0; }
channelAttributes: /*null*/ | channelAttributes channelAttribute
channelAttribute:
    EQTOKEN_HINT_STATISTICS IATTR
        { channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, $2 ); }


layout: EQTOKEN_LAYOUT '{' { layout = new eq::server::Layout; }
            layoutFields '}' { config->addLayout( layout ); layout = 0; }
layoutFields: /*null*/ | layoutFields layoutField
layoutField:
    EQTOKEN_NAME STRING { layout->setName( $2 ); }
    | view

view: EQTOKEN_VIEW '{' { view = new eq::server::View; }
          viewFields '}' { layout->addView( view ); view = 0; }
viewFields: /*null*/ | viewFields viewField
viewField:
    EQTOKEN_NAME STRING { view->setName( $2 ); }
    | EQTOKEN_VIEWPORT viewport
        { view->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] ));}
    | wall       { view->setWall( wall ); }
    | projection { view->setProjection( projection ); }

canvas: EQTOKEN_CANVAS '{' { canvas = new eq::server::Canvas; }
            canvasFields '}' { config->addCanvas( canvas ); canvas = 0; }
canvasFields: /*null*/ | canvasFields canvasField
canvasField:
    EQTOKEN_NAME STRING { canvas->setName( $2 ); }
    | EQTOKEN_LAYOUT STRING 
      {
          eq::server::Layout* layout = config->findLayout( $2 );
          if( !layout )
              yyerror( "No layout of the given name" );
          else
              canvas->useLayout( layout ); 
      }
    | EQTOKEN_LAYOUT UNSIGNED
      {
          const eq::server::LayoutPath path( $2 );
          eq::server::Layout* layout = config->getLayout( path );
          if( !layout )
              yyerror( "No layout of the given index" );
          else
              canvas->useLayout( layout ); 
      }
    | wall       { canvas->setWall( wall ); }
    | projection { canvas->setProjection( projection ); }
    | segment

segment: EQTOKEN_SEGMENT '{' { segment = new eq::server::Segment; }
          segmentFields '}' { canvas->addSegment( segment ); segment = 0; }
segmentFields: /*null*/ | segmentFields segmentField
segmentField:
    EQTOKEN_NAME STRING { segment->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
        {
            eq::server::Channel* channel = config->findChannel( $2 );
            if( !channel )
                yyerror( "No channel of the given name" );
            else
                segment->setChannel( channel );
        }
    | EQTOKEN_VIEWPORT viewport
        { segment->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] ));}
    | wall       { segment->setWall( wall ); }
    | projection { segment->setProjection( projection ); }


compound: EQTOKEN_COMPOUND '{' 
              {
                  eq::server::Compound* child = new eq::server::Compound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              }
          compoundFields 
          '}' { eqCompound = eqCompound->getParent(); } 

compoundFields: /*null*/ | compoundFields compoundField
compoundField: 
    compound
    | EQTOKEN_NAME STRING { eqCompound->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
      {
          eq::server::Channel* channel = config->findChannel( $2 );
          if( !channel )
              yyerror( "No channel of the given name" );
          else
              eqCompound->setChannel( channel );
      }
    | EQTOKEN_CHANNEL viewSegmentRef
      {
          if( !segment || !view )
              yyerror( "Incomplete channel reference (view or segment missing)" );
          else
          {
              eq::server::Channel* channel = config->findChannel(segment, view);
              if( channel )
                  eqCompound->setChannel( channel );
              else
                  yyerror( "No channel for the given view and segment" );
          }

          canvas = 0;
          segment = 0;
          layout = 0;
          view = 0;
      }
    | EQTOKEN_TASK '['   { eqCompound->setTasks( eq::TASK_NONE ); }
        compoundTasks ']'
    | EQTOKEN_EYE  '['   { eqCompound->setEyes( eq::server::Compound::EYE_UNDEFINED );}
        compoundEyes  ']'
    | EQTOKEN_BUFFER '[' { flags = eq::Frame::BUFFER_NONE; }
        buffers ']' { eqCompound->setBuffers( flags ); flags = 0; }
    | EQTOKEN_VIEWPORT viewport
        { eqCompound->setViewport( eq::Viewport( $2[0], $2[1], $2[2], $2[3] ));}
    | EQTOKEN_RANGE '[' FLOAT FLOAT ']'
        { eqCompound->setRange( eq::Range( $3, $4 )); }
    | EQTOKEN_PERIOD UNSIGNED { eqCompound->setPeriod( $2 ); }
    | EQTOKEN_PHASE  UNSIGNED { eqCompound->setPhase( $2 ); }
    | EQTOKEN_ZOOM '[' FLOAT FLOAT ']'
        { eqCompound->setZoom( eq::Zoom( $3, $4 )); }
    | EQTOKEN_PIXEL '[' UNSIGNED UNSIGNED UNSIGNED UNSIGNED ']'
        { eqCompound->setPixel( eq::Pixel( $3, $4, $5, $6 )); }
    | wall { eqCompound->setWall( wall ); }
    | projection { eqCompound->setProjection( projection ); }
    | loadBalancer
    | swapBarrier
    | outputFrame
    | inputFrame
    | EQTOKEN_ATTRIBUTES '{' compoundAttributes '}'

viewSegmentRef: 
    '(' {
            canvas = 0;
            segment = 0;
            layout = 0;
            view = 0;
        }
    viewSegmentRefFields ')'

viewSegmentRefFields : /*null*/ | viewSegmentRefFields viewSegmentRefField
viewSegmentRefField:
    EQTOKEN_CANVAS STRING 
        { canvas = config->findCanvas( $2 ); }
    | EQTOKEN_CANVAS UNSIGNED 
        { canvas = config->getCanvas( eq::server::CanvasPath( $2 )); }
    | EQTOKEN_SEGMENT STRING 
        { 
            if( canvas )
                segment = canvas->findSegment( $2 ); 
            else
                segment = config->findSegment( $2 );
        }
    | EQTOKEN_SEGMENT UNSIGNED 
        {
            if( canvas )
                segment = canvas->getSegment( eq::server::SegmentPath( $2 ));
            else
                segment = config->getSegment( eq::server::SegmentPath( $2 ));
        }
    | EQTOKEN_LAYOUT STRING 
        { layout = config->findLayout( $2 ); }
    | EQTOKEN_LAYOUT UNSIGNED 
        { layout = config->getLayout( eq::server::LayoutPath( $2 )); }
    | EQTOKEN_VIEW STRING 
        { 
            if( layout )
                view = layout->findView( $2 ); 
            else
                view = config->findView( $2 );
        }
    | EQTOKEN_VIEW UNSIGNED 
        {
            if( layout )
                view = layout->getView( eq::server::ViewPath( $2 ));
            else
                view = config->getView( eq::server::ViewPath( $2 ));
        }

compoundTasks: /*null*/ | compoundTasks compoundTask
compoundTask:
    EQTOKEN_CLEAR      { eqCompound->enableTask( eq::TASK_CLEAR ); }
    | EQTOKEN_DRAW     { eqCompound->enableTask( eq::TASK_DRAW ); }
    | EQTOKEN_ASSEMBLE { eqCompound->enableTask( eq::TASK_ASSEMBLE );}
    | EQTOKEN_READBACK { eqCompound->enableTask( eq::TASK_READBACK );}

compoundEyes: /*null*/ | compoundEyes compoundEye
compoundEye:
    EQTOKEN_CYCLOP  { eqCompound->enableEye( eq::server::Compound::EYE_CYCLOP_BIT ); }
    | EQTOKEN_LEFT  { eqCompound->enableEye( eq::server::Compound::EYE_LEFT_BIT ); }
    | EQTOKEN_RIGHT { eqCompound->enableEye( eq::server::Compound::EYE_RIGHT_BIT ); }

buffers: /*null*/ | buffers buffer
buffer:
    EQTOKEN_COLOR    { flags |= eq::Frame::BUFFER_COLOR; }
    | EQTOKEN_DEPTH  { flags |= eq::Frame::BUFFER_DEPTH; }
    
drawables:  /*null*/ | drawables drawable
drawable:  
    EQTOKEN_FBO_COLOR     { flags |= eq::Channel::FBO_COLOR; }
    | EQTOKEN_FBO_DEPTH   { flags |= eq::Channel::FBO_DEPTH; }
    | EQTOKEN_FBO_STENCIL { flags |= eq::Channel::FBO_STENCIL; }

wall: EQTOKEN_WALL '{' { wall = eq::Wall(); } wallFields '}'

wallFields:  /*null*/ | wallFields wallField
wallField:
    EQTOKEN_BOTTOM_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomLeft = vmml::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_BOTTOM_RIGHT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomRight = vmml::Vector3f( $3, $4, $5 ); }
   |  EQTOKEN_TOP_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.topLeft = vmml::Vector3f( $3, $4, $5 ); }

projection: EQTOKEN_PROJECTION '{' { projection = eq::Projection(); } 
                projectionFields '}'

projectionFields:  /*null*/ | projectionFields projectionField
projectionField:
    EQTOKEN_ORIGIN  '[' FLOAT FLOAT FLOAT ']'
        { projection.origin = vmml::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_DISTANCE FLOAT
        { projection.distance = $2; }
    | EQTOKEN_FOV  '[' FLOAT FLOAT ']'
        { projection.fov = vmml::Vector2f( $3, $4 ); }
    | EQTOKEN_HPR  '[' FLOAT FLOAT FLOAT ']'
        { projection.hpr = vmml::Vector3f( $3, $4, $5 ); }

loadBalancer: EQTOKEN_LOADBALANCER 
    '{' { EQASSERT( !loadBalancer ); loadBalancer = new eq::server::LoadBalancer(); }
         loadBalancerFields
    '}' { eqCompound->addLoadBalancer( loadBalancer ); loadBalancer = 0; }

loadBalancerFields: /*null*/ | loadBalancerFields loadBalancerField
loadBalancerField:
    EQTOKEN_MODE loadBalancerMode { loadBalancer->setMode( $2 ) }
    | EQTOKEN_DAMPING FLOAT       { loadBalancer->setDamping( $2 ) }
    | EQTOKEN_FRAMERATE FLOAT     { loadBalancer->setFrameRate( $2 ) }

loadBalancerMode: EQTOKEN_2D { $$ = eq::server::LoadBalancer::MODE_2D; }
    | EQTOKEN_DB             { $$ = eq::server::LoadBalancer::MODE_DB; }
    | EQTOKEN_HORIZONTAL     { $$ = eq::server::LoadBalancer::MODE_HORIZONTAL; }
    | EQTOKEN_VERTICAL       { $$ = eq::server::LoadBalancer::MODE_VERTICAL; }
    | EQTOKEN_DPLEX          { $$ = eq::server::LoadBalancer::MODE_DPLEX; }
    | EQTOKEN_DFR            { $$ = eq::server::LoadBalancer::MODE_DFR; } 
    | EQTOKEN_DDS            { $$ = eq::server::LoadBalancer::MODE_DDS; } 
    

swapBarrier: EQTOKEN_SWAPBARRIER '{' { swapBarrier = new eq::server::SwapBarrier; }
    swapBarrierFields '}'
        { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        } 
swapBarrierFields: /*null*/ | swapBarrierFields swapBarrierField
swapBarrierField: EQTOKEN_NAME STRING { swapBarrier->setName( $2 ); }
    | EQTOKEN_NVGROUP IATTR { swapBarrier->setNVSwapGroup( $2 ); }
    | EQTOKEN_NVBARRIER IATTR { swapBarrier->setNVSwapBarrier( $2 ); }
    


outputFrame : EQTOKEN_OUTPUTFRAME '{' { frame = new eq::server::Frame; }
    frameFields '}'
        { 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        } 
inputFrame: EQTOKEN_INPUTFRAME '{' { frame = new eq::server::Frame; }
    frameFields '}'
        { 
            eqCompound->addInputFrame( frame );
            frame = 0;
        } 
frameFields: /*null*/ | frameFields frameField
frameField: 
    EQTOKEN_NAME STRING { frame->setName( $2 ); }
    | EQTOKEN_TYPE frameType
    | EQTOKEN_VIEWPORT viewport
        { frame->setViewport(eq::Viewport( $2[0], $2[1], $2[2], $2[3])); }
    | EQTOKEN_BUFFER '[' { flags = eq::Frame::BUFFER_NONE; }
        buffers ']' { frame->setBuffers( flags ); flags = 0; }
    | EQTOKEN_ZOOM '[' FLOAT FLOAT ']'
        { frame->setZoom( eq::Zoom( $3, $4 )); }

frameType: 
    EQTOKEN_TEXTURE { frame->setType( eq::Frame::TYPE_TEXTURE ); }
    | EQTOKEN_MEMORY { frame->setType( eq::Frame::TYPE_MEMORY ); }

compoundAttributes: /*null*/ | compoundAttributes compoundAttribute
compoundAttribute:
    EQTOKEN_STEREO_MODE IATTR 
        { eqCompound->setIAttribute( eq::server::Compound::IATTR_STEREO_MODE, $2 ); }
    | EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK colorMask
        { eqCompound->setIAttribute( 
                eq::server::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, $2 ); }
    | EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK colorMask
        { eqCompound->setIAttribute( 
                eq::server::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, $2 ); }
    | EQTOKEN_UPDATE_FOV IATTR
        { EQWARN << "ignoring removed attribute update_FOV" << std::endl; }
    | EQTOKEN_HINT_OFFSET IATTR
        { eqCompound->setIAttribute(eq::server::Compound::IATTR_HINT_OFFSET, $2 ); }

viewport: '[' FLOAT FLOAT FLOAT FLOAT ']'
     { 
         $$[0] = $2;
         $$[1] = $3;
         $$[2] = $4;
         $$[3] = $5;
     }

colorMask: '[' colorMaskBits ']' { $$ = $2; }
colorMaskBits: 
    /*null*/ { $$ =eq::server::Compound::COLOR_MASK_NONE; }
    | colorMaskBit { $$ = $1; }
    | colorMaskBits colorMaskBit { $$ = ($1 | $2);}
colorMaskBit:
    EQTOKEN_RED     { $$ = eq::server::Compound::COLOR_MASK_RED; }
    | EQTOKEN_GREEN { $$ = eq::server::Compound::COLOR_MASK_GREEN; }
    | EQTOKEN_BLUE  { $$ = eq::server::Compound::COLOR_MASK_BLUE; }

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
    | EQTOKEN_FBO        { $$ = eq::FBO; }
    | EQTOKEN_PBUFFER    { $$ = eq::PBUFFER; }
    | EQTOKEN_ASYNC      { $$ = eq::ASYNC; }
    | EQTOKEN_DRAW_SYNC  { $$ = eq::DRAW_SYNC; }
    | EQTOKEN_LOCAL_SYNC { $$ = eq::LOCAL_SYNC; }
    | INTEGER            { $$ = $1; }

STRING: EQTOKEN_STRING
     {
         static std::string stringBuf;
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
            << " at '" << yytext << "'" << std::endl;
}

namespace eq
{
namespace server
{

//---------------------------------------------------------------------------
// loader
//---------------------------------------------------------------------------
ServerPtr Loader::loadFile( const std::string& filename )
{
    EQASSERTINFO( !eq::loader::loader, "Config file loader is not reentrant" );
    eq::loader::loader = this;

    yyin       = fopen( filename.c_str(), "r" );
    yyinString = 0;

    if( !yyin )
    {
        EQERROR << "Can't open config file " << filename << std::endl;
        eq::loader::loader = 0;
        return 0;
    }

    loader::server = 0;
    config = 0;
    eqLoader_parse();

    fclose( yyin );
    eq::loader::loader = 0;

    if( loader::server.isValid( ))
    {
        const server::ConfigVector& configs = loader::server->getConfigs();
        for( server::ConfigVector::const_iterator i = configs.begin();
             i != configs.end(); ++i )
        {
            Config* config = *i;
            if( config->getName().empty( ))
                config->setName( filename );
        }
    }

    eq::server::ServerPtr server = loader::server;
    loader::server = 0;
    return server;
}

void Loader::_parseString( const char* data )
{
    EQASSERTINFO( !eq::loader::loader, "Config file loader is not reentrant" );
    eq::loader::loader = this;

    yyin       = 0;
    yyinString = data;

    loader::server = 0;
    config = 0;
    eqLoader_parse();

    eq::loader::loader = 0;
}

Config* Loader::parseConfig( const char* data )
{
    _parseString( data );
    return config;
}

ServerPtr Loader::parseServer( const char* data )
{
    _parseString( data );

    eq::server::ServerPtr server = loader::server;
    loader::server = 0;
    return server;
}

}
}
