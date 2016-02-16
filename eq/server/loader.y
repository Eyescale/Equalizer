
/* Copyright (c) 2006-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

%{
#include "loader.h"

#include "canvas.h"
#include "channel.h"
#include "compound.h"
#include "equalizers/dfrEqualizer.h"
#include "equalizers/framerateEqualizer.h"
#include "equalizers/loadEqualizer.h"
#include "equalizers/treeEqualizer.h"
#include "equalizers/monitorEqualizer.h"
#include "equalizers/viewEqualizer.h"
#include "equalizers/tileEqualizer.h"
#include "frame.h"
#include "tileQueue.h"
#include "global.h"
#include "layout.h"
#include "node.h"
#include "observer.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

#include <eq/fabric/paths.h>
#include <lunchbox/os.h>
#include <lunchbox/file.h>

#include <locale.h>
#include <string>

#pragma warning(disable: 4065)

    namespace eq
    {
    namespace loader
    {
        static eq::server::Loader*      loader = 0;
        std::string filename;

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
        static eq::server::Observer*    observer = 0;
        static eq::server::Compound*    eqCompound = 0; // avoid name clash
        static eq::server::DFREqualizer* dfrEqualizer = 0;
        static eq::server::LoadEqualizer* loadEqualizer = 0;
        static eq::server::TreeEqualizer* treeEqualizer = 0;
        static eq::server::TileEqualizer* tileEqualizer = 0;
        static eq::server::SwapBarrierPtr swapBarrier;
        static eq::server::Frame*       frame = 0;
        static eq::server::TileQueue*   tileQueue = 0;
        static co::ConnectionDescriptionPtr connectionDescription;
        static eq::fabric::Wall         wall;
        static eq::fabric::Projection   projection;
        static uint32_t                 flags = 0;
    }
    }

    using namespace lunchbox;
    using namespace eq::loader;

    int eqLoader_lex();

    #define yylineno eqLoader_lineno
    void yyerror( const char *errmsg );
    extern char* yytext;
    extern FILE*       yyin;
    extern const char* yyinString;
    extern int yylineno;
%}

%token EQTOKEN_HEADER
%token EQTOKEN_ASCII
%token EQTOKEN_GLOBAL
%token EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS
%token EQTOKEN_CHANNEL_IATTR_HINT_SENDTOKEN
%token EQTOKEN_CHANNEL_SATTR_DUMP_IMAGE
%token EQTOKEN_COMPOUND_IATTR_STEREO_MODE
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_COMPOUND_IATTR_UPDATE_FOV
%token EQTOKEN_CONNECTION_SATTR_FILENAME
%token EQTOKEN_CONNECTION_SATTR_HOSTNAME
%token EQTOKEN_CONNECTION_IATTR_BANDWIDTH
%token EQTOKEN_CONNECTION_IATTR_TYPE
%token EQTOKEN_CONNECTION_IATTR_PORT
%token EQTOKEN_CONFIG_FATTR_EYE_BASE
%token EQTOKEN_CONFIG_IATTR_ROBUSTNESS
%token EQTOKEN_NODE_SATTR_LAUNCH_COMMAND
%token EQTOKEN_NODE_CATTR_LAUNCH_COMMAND_QUOTE
%token EQTOKEN_NODE_IATTR_THREAD_MODEL
%token EQTOKEN_NODE_IATTR_HINT_AFFINITY
%token EQTOKEN_NODE_IATTR_HINT_STATISTICS
%token EQTOKEN_NODE_IATTR_LAUNCH_TIMEOUT
%token EQTOKEN_PIPE_IATTR_HINT_CUDA_GL_INTEROP
%token EQTOKEN_PIPE_IATTR_HINT_THREAD
%token EQTOKEN_PIPE_IATTR_HINT_AFFINITY
%token EQTOKEN_VIEW_SATTR_DISPLAYCLUSTER
%token EQTOKEN_WINDOW_IATTR_HINT_CORE_PROFILE
%token EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MAJOR
%token EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MINOR
%token EQTOKEN_WINDOW_IATTR_HINT_STEREO
%token EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER
%token EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN
%token EQTOKEN_WINDOW_IATTR_HINT_DECORATION
%token EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC
%token EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE
%token EQTOKEN_WINDOW_IATTR_HINT_STATISTICS
%token EQTOKEN_WINDOW_IATTR_HINT_SCREENSAVER
%token EQTOKEN_WINDOW_IATTR_HINT_GRAB_POINTER
%token EQTOKEN_WINDOW_IATTR_PLANES_ACCUM
%token EQTOKEN_WINDOW_IATTR_PLANES_ACCUM_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_ALPHA
%token EQTOKEN_WINDOW_IATTR_PLANES_COLOR
%token EQTOKEN_WINDOW_IATTR_PLANES_DEPTH
%token EQTOKEN_WINDOW_IATTR_PLANES_SAMPLES
%token EQTOKEN_WINDOW_IATTR_PLANES_STENCIL
%token EQTOKEN_SERVER
%token EQTOKEN_CONFIG
%token EQTOKEN_APPNODE
%token EQTOKEN_NODE
%token EQTOKEN_PIPE
%token EQTOKEN_WINDOW
%token EQTOKEN_ATTRIBUTES
%token EQTOKEN_HINT_CORE_PROFILE
%token EQTOKEN_HINT_OPENGL_MAJOR
%token EQTOKEN_HINT_OPENGL_MINOR
%token EQTOKEN_HINT_STEREO
%token EQTOKEN_HINT_DOUBLEBUFFER
%token EQTOKEN_HINT_FULLSCREEN
%token EQTOKEN_HINT_DECORATION
%token EQTOKEN_HINT_STATISTICS
%token EQTOKEN_HINT_SENDTOKEN
%token EQTOKEN_HINT_SWAPSYNC
%token EQTOKEN_HINT_DRAWABLE
%token EQTOKEN_HINT_THREAD
%token EQTOKEN_HINT_AFFINITY
%token EQTOKEN_HINT_CUDA_GL_INTEROP
%token EQTOKEN_HINT_SCREENSAVER
%token EQTOKEN_HINT_GRAB_POINTER
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
%token EQTOKEN_PASSIVE
%token EQTOKEN_RED
%token EQTOKEN_GREEN
%token EQTOKEN_BLUE
%token EQTOKEN_HORIZONTAL
%token EQTOKEN_VERTICAL
%token EQTOKEN_FRAMERATE
%token EQTOKEN_CHANNEL
%token EQTOKEN_OBSERVER
%token EQTOKEN_LAYOUT
%token EQTOKEN_VIEW
%token EQTOKEN_CANVAS
%token EQTOKEN_SEGMENT
%token EQTOKEN_COMPOUND
%token EQTOKEN_DFREQUALIZER
%token EQTOKEN_FRAMERATEEQUALIZER
%token EQTOKEN_LOADEQUALIZER
%token EQTOKEN_TREEEQUALIZER
%token EQTOKEN_MONITOREQUALIZER
%token EQTOKEN_VIEWEQUALIZER
%token EQTOKEN_TILEEQUALIZER
%token EQTOKEN_DAMPING
%token EQTOKEN_CONNECTION
%token EQTOKEN_NAME
%token EQTOKEN_TYPE
%token EQTOKEN_TCPIP
%token EQTOKEN_SDP
%token EQTOKEN_RSP
%token EQTOKEN_RDMA
%token EQTOKEN_UDT
%token EQTOKEN_TEXTURE
%token EQTOKEN_MEMORY
%token EQTOKEN_FIXED
%token EQTOKEN_RELATIVE_TO_ORIGIN
%token EQTOKEN_RELATIVE_TO_OBSERVER
%token EQTOKEN_HMD
%token EQTOKEN_HOST
%token EQTOKEN_HOSTNAME
%token EQTOKEN_INTERFACE
%token EQTOKEN_LAUNCH_COMMAND
%token EQTOKEN_LAUNCH_COMMAND_QUOTE
%token EQTOKEN_LAUNCH_TIMEOUT
%token EQTOKEN_PORT
%token EQTOKEN_FILENAME
%token EQTOKEN_TASK
%token EQTOKEN_EYE
%token EQTOKEN_EYE_BASE
%token EQTOKEN_EYE_LEFT
%token EQTOKEN_EYE_CYCLOP
%token EQTOKEN_EYE_RIGHT
%token EQTOKEN_FOCUS_DISTANCE
%token EQTOKEN_FOCUS_MODE
%token EQTOKEN_OPENCV_CAMERA
%token EQTOKEN_VRPN_TRACKER
%token EQTOKEN_ROBUSTNESS
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
%token EQTOKEN_RANGE
%token EQTOKEN_PERIOD
%token EQTOKEN_PHASE
%token EQTOKEN_PIXEL
%token EQTOKEN_SUBPIXEL
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
%token EQTOKEN_OUTPUTTILES
%token EQTOKEN_INPUTTILES
%token EQTOKEN_STEREO_MODE
%token EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK
%token EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK
%token EQTOKEN_UPDATE_FOV
%token EQTOKEN_PBUFFER
%token EQTOKEN_FBO
%token EQTOKEN_RGBA16F
%token EQTOKEN_RGBA32F
%token EQTOKEN_MODE
%token EQTOKEN_2D
%token EQTOKEN_ASSEMBLE_ONLY_LIMIT
%token EQTOKEN_DB
%token EQTOKEN_BOUNDARY
%token EQTOKEN_RESISTANCE
%token EQTOKEN_ZOOM
%token EQTOKEN_MONO
%token EQTOKEN_STEREO
%token EQTOKEN_STRING
%token EQTOKEN_CHARACTER
%token EQTOKEN_FLOAT
%token EQTOKEN_INTEGER
%token EQTOKEN_UNSIGNED
%token EQTOKEN_SIZE
%token EQTOKEN_CORE
%token EQTOKEN_SOCKET
%token EQTOKEN_DISPLAYCLUSTER
%token EQTOKEN_DUMP_IMAGE

%union{
    const char*             _string;
    char                    _character;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    co::ConnectionType   _connectionType;
    eq::server::LoadEqualizer::Mode _loadEqualizerMode;
    eq::server::TreeEqualizer::Mode _treeEqualizerMode;
    float                   _viewport[4];
}

%type <_string>           STRING;
%type <_character>        CHARACTER;
%type <_int>              INTEGER IATTR;
%type <_unsigned>         UNSIGNED colorMask colorMaskBit colorMaskBits;
%type <_connectionType>   connectionType;
%type <_loadEqualizerMode> loadEqualizerMode;
%type <_treeEqualizerMode> treeEqualizerMode;
%type <_viewport>         viewport;
%type <_float>            FLOAT;

%%

file:   header global server | header global config;

header: /* null */ | EQTOKEN_HEADER FLOAT EQTOKEN_ASCII
    {
        eq::server::Global::instance()->setConfigFAttribute(
            eq::server::Config::FATTR_VERSION, $2 );
    }

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
     | EQTOKEN_CONNECTION_IATTR_TYPE connectionType
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_TYPE, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_PORT UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_PORT, $2 );
     }
     | EQTOKEN_CONNECTION_SATTR_FILENAME STRING
     {
         eq::server::Global::instance()->setConnectionSAttribute(
             eq::server::ConnectionDescription::SATTR_FILENAME, $2 );
     }
     | EQTOKEN_CONNECTION_IATTR_BANDWIDTH UNSIGNED
     {
         eq::server::Global::instance()->setConnectionIAttribute(
             eq::server::ConnectionDescription::IATTR_BANDWIDTH, $2 );
     }
     | EQTOKEN_CONFIG_FATTR_EYE_BASE FLOAT
     {
         eq::server::Global::instance()->setConfigFAttribute(
             eq::server::Config::FATTR_EYE_BASE, $2 );
     }
     | EQTOKEN_CONFIG_IATTR_ROBUSTNESS IATTR
     {
         eq::server::Global::instance()->setConfigIAttribute(
             eq::server::Config::IATTR_ROBUSTNESS, $2 );
     }
     | EQTOKEN_NODE_SATTR_LAUNCH_COMMAND STRING
     {
         eq::server::Global::instance()->setNodeSAttribute(
             eq::server::Node::SATTR_LAUNCH_COMMAND, $2 );
     }
     | EQTOKEN_NODE_CATTR_LAUNCH_COMMAND_QUOTE CHARACTER
     {
         eq::server::Global::instance()->setNodeCAttribute(
             eq::server::Node::CATTR_LAUNCH_COMMAND_QUOTE, $2 );
     }
     | EQTOKEN_NODE_IATTR_THREAD_MODEL IATTR
     {
         eq::server::Global::instance()->setNodeIAttribute(
             eq::server::Node::IATTR_THREAD_MODEL, $2 );
     }
     | EQTOKEN_NODE_IATTR_HINT_AFFINITY IATTR
     {
         eq::server::Global::instance()->setNodeIAttribute(
             eq::server::Node::IATTR_HINT_AFFINITY, $2 );
     }
     | EQTOKEN_NODE_IATTR_LAUNCH_TIMEOUT UNSIGNED
     {
         eq::server::Global::instance()->setNodeIAttribute(
             eq::server::Node::IATTR_LAUNCH_TIMEOUT, $2 );
     }
     | EQTOKEN_NODE_IATTR_HINT_STATISTICS IATTR
     {
         LBWARN << "Ignoring deprecated attribute Node::IATTR_HINT_STATISTICS"
                << std::endl;
     }
     | EQTOKEN_PIPE_IATTR_HINT_THREAD IATTR
     {
         eq::server::Global::instance()->setPipeIAttribute(
             eq::server::Pipe::IATTR_HINT_THREAD, $2 );
     }
     | EQTOKEN_PIPE_IATTR_HINT_AFFINITY IATTR
     {
         eq::server::Global::instance()->setPipeIAttribute(
             eq::server::Pipe::IATTR_HINT_AFFINITY, $2 );
     }
     | EQTOKEN_PIPE_IATTR_HINT_CUDA_GL_INTEROP IATTR
     {
         eq::server::Global::instance()->setPipeIAttribute(
             eq::server::Pipe::IATTR_HINT_CUDA_GL_INTEROP, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_CORE_PROFILE IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_CORE_PROFILE, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MAJOR IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_OPENGL_MAJOR, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MINOR IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_OPENGL_MINOR, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STEREO IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_STEREO, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_DOUBLEBUFFER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_FULLSCREEN, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DECORATION IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_DECORATION, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_SWAPSYNC, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_DRAWABLE, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_STATISTICS IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_SCREENSAVER IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_SCREENSAVER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_HINT_GRAB_POINTER IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_HINT_GRAB_POINTER, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_COLOR IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_COLOR, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ALPHA IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_ALPHA, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_DEPTH IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_DEPTH, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_STENCIL IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_STENCIL, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ACCUM IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_ACCUM, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_ACCUM_ALPHA IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_ACCUM_ALPHA, $2 );
     }
     | EQTOKEN_WINDOW_IATTR_PLANES_SAMPLES IATTR
     {
         eq::server::Global::instance()->setWindowIAttribute(
             eq::server::WindowSettings::IATTR_PLANES_SAMPLES, $2 );
     }
     | EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS IATTR
     {
         eq::server::Global::instance()->setChannelIAttribute(
             eq::server::Channel::IATTR_HINT_STATISTICS, $2 );
     }
     | EQTOKEN_CHANNEL_IATTR_HINT_SENDTOKEN IATTR
     {
         eq::server::Global::instance()->setChannelIAttribute(
             eq::server::Channel::IATTR_HINT_SENDTOKEN, $2 );
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
         LBWARN << "ignoring removed attribute EQ_COMPOUND_IATTR_UPDATE_FOV"
                << std::endl;
     }
     | EQTOKEN_CHANNEL_SATTR_DUMP_IMAGE STRING
     {
        eq::server::Global::instance()->setChannelSAttribute(
            eq::server::Channel::SATTR_DUMP_IMAGE, $2 );
     }
     | EQTOKEN_VIEW_SATTR_DISPLAYCLUSTER STRING
     {
        eq::server::Global::instance()->setViewSAttribute(
            eq::server::View::SATTR_DISPLAYCLUSTER, $2 );
     }

connectionType:
    EQTOKEN_TCPIP  { $$ = co::CONNECTIONTYPE_TCPIP; }
    | EQTOKEN_SDP  { $$ = co::CONNECTIONTYPE_SDP; }
    | EQTOKEN_PIPE { $$ = co::CONNECTIONTYPE_NAMEDPIPE; }
    | EQTOKEN_RSP  { $$ = co::CONNECTIONTYPE_RSP; }
    | EQTOKEN_RDMA { $$ = co::CONNECTIONTYPE_RDMA; }
    | EQTOKEN_UDT  { $$ = co::CONNECTIONTYPE_UDT; }

server: EQTOKEN_SERVER '{' { server = new eq::server::Server(); }
        serverConnections
        configs '}'

serverConnections: /*null*/ | serverConnections serverConnection
serverConnection: EQTOKEN_CONNECTION
        '{' {
                connectionDescription = new eq::server::ConnectionDescription;
                connectionDescription->setHostname( "" );
                connectionDescription->port = 0; // OS chosen port
            }
            connectionFields '}'
            {
                server->addConnectionDescription( connectionDescription.get( ));
                connectionDescription = 0;
            }

configs: config | configs config
config: EQTOKEN_CONFIG '{'
            {
                config = new eq::server::Config( server );
                config->setName( filename );
                node = new eq::server::Node( config );
                node->setApplicationNode( true );
            }
        configFields '}' { config = 0; }
configFields: /*null*/ | configFields configField
configField:
    node
    | EQTOKEN_NAME STRING       { config->setName( $2 ); }
    | observer
    | layout
    | canvas
    | compound
    | EQTOKEN_LATENCY UNSIGNED  { config->setLatency( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' configAttributes '}'
configAttributes: /*null*/ | configAttributes configAttribute
configAttribute:
    EQTOKEN_EYE_BASE FLOAT { config->setFAttribute(
                             eq::server::Config::FATTR_EYE_BASE, $2 ); }
    | EQTOKEN_ROBUSTNESS IATTR { config->setIAttribute(
                                 eq::server::Config::IATTR_ROBUSTNESS, $2 ); }

node: appNode | renderNode
renderNode: EQTOKEN_NODE '{' {
                                 node = new eq::server::Node( config );
                             }
               nodeFields
               '}' { node = 0; }
appNode: EQTOKEN_APPNODE '{'
            {
                node = config->findApplicationNode();
                LBASSERT( node );
            }
            nodeFields
            '}' { node = 0; }
nodeFields: /*null*/ | nodeFields nodeField
nodeField:
    EQTOKEN_NAME STRING            { node->setName( $2 ); }
    | EQTOKEN_HOST STRING          { node->setHost( $2 ); }
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
    | EQTOKEN_INTERFACE STRING    { connectionDescription->setInterface($2); }
    | EQTOKEN_PORT UNSIGNED       { connectionDescription->port = $2; }
    | EQTOKEN_BANDWIDTH UNSIGNED  { connectionDescription->bandwidth = $2; }
    | EQTOKEN_FILENAME STRING     { connectionDescription->setFilename($2); }

nodeAttributes: /*null*/ | nodeAttributes nodeAttribute
nodeAttribute:
    EQTOKEN_LAUNCH_COMMAND STRING
        { node->setSAttribute( eq::server::Node::SATTR_LAUNCH_COMMAND, $2 ); }
    | EQTOKEN_LAUNCH_COMMAND_QUOTE CHARACTER
        { node->setCAttribute( eq::server::Node::CATTR_LAUNCH_COMMAND_QUOTE,
                               $2 ); }
    | EQTOKEN_THREAD_MODEL IATTR
        { node->setIAttribute( eq::server::Node::IATTR_THREAD_MODEL, $2 ); }
    | EQTOKEN_LAUNCH_TIMEOUT IATTR
        { node->setIAttribute( eq::server::Node::IATTR_LAUNCH_TIMEOUT, $2 ); }
    | EQTOKEN_HINT_STATISTICS IATTR
        {
            LBWARN
                << "Ignoring deprecated attribute Node::IATTR_HINT_STATISTICS"
                << std::endl;
        }
    | EQTOKEN_HINT_AFFINITY IATTR
        { node->setIAttribute( eq::server::Node::IATTR_HINT_AFFINITY, $2 ); }


pipe: EQTOKEN_PIPE '{'
            {
                eqPipe = new eq::server::Pipe( node );
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
            eqPipe->setPixelViewport( eq::fabric::PixelViewport
                        ( (int)$2[0], (int)$2[1], (int)$2[2], (int)$2[3] ));
        }
pipeAttributes: /*null*/ | pipeAttributes pipeAttribute
pipeAttribute:
    EQTOKEN_HINT_THREAD IATTR
        { eqPipe->setIAttribute( eq::server::Pipe::IATTR_HINT_THREAD, $2 ); }
    | EQTOKEN_HINT_AFFINITY IATTR
        { eqPipe->setIAttribute( eq::server::Pipe::IATTR_HINT_AFFINITY, $2); }
    | EQTOKEN_HINT_CUDA_GL_INTEROP IATTR
        { eqPipe->setIAttribute( eq::server::Pipe::IATTR_HINT_CUDA_GL_INTEROP,
                                 $2 ); }

window: EQTOKEN_WINDOW '{'
            {
                window = new eq::server::Window( eqPipe );
                window->init(); // not in ctor, virtual method
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
                window->setPixelViewport( eq::fabric::PixelViewport
                        ( (int)$2[0], (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                window->setViewport( eq::fabric::Viewport($2[0], $2[1],
                                                          $2[2], $2[3]));
        }
windowAttributes: /*null*/ | windowAttributes windowAttribute
windowAttribute:
    EQTOKEN_HINT_CORE_PROFILE IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_CORE_PROFILE, $2 ); }
    | EQTOKEN_HINT_OPENGL_MAJOR IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_OPENGL_MAJOR, $2 ); }
    | EQTOKEN_HINT_OPENGL_MINOR IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_OPENGL_MINOR, $2 ); }
    | EQTOKEN_HINT_STEREO IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_STEREO, $2 ); }
    | EQTOKEN_HINT_DOUBLEBUFFER IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_DOUBLEBUFFER, $2 ); }
    | EQTOKEN_HINT_FULLSCREEN IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_FULLSCREEN, $2 ); }
    | EQTOKEN_HINT_DECORATION IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_DECORATION, $2 ); }
    | EQTOKEN_HINT_SWAPSYNC IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_SWAPSYNC, $2 ); }
    | EQTOKEN_HINT_DRAWABLE IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_DRAWABLE, $2 ); }
    | EQTOKEN_HINT_STATISTICS IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_STATISTICS, $2 ); }
    | EQTOKEN_HINT_SCREENSAVER IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_SCREENSAVER, $2 ); }
    | EQTOKEN_HINT_GRAB_POINTER IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_HINT_GRAB_POINTER, $2 ); }
    | EQTOKEN_PLANES_COLOR IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_COLOR, $2 ); }
    | EQTOKEN_PLANES_ALPHA IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_ALPHA, $2 ); }
    | EQTOKEN_PLANES_DEPTH IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_DEPTH, $2 ); }
    | EQTOKEN_PLANES_STENCIL IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_STENCIL, $2 ); }
    | EQTOKEN_PLANES_ACCUM IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_ACCUM, $2 ); }
    | EQTOKEN_PLANES_ACCUM_ALPHA IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_ACCUM_ALPHA, $2 ); }
    | EQTOKEN_PLANES_SAMPLES IATTR
        { window->setIAttribute( eq::server::WindowSettings::IATTR_PLANES_SAMPLES, $2 ); }

channel: EQTOKEN_CHANNEL '{'
            {
                channel = new eq::server::Channel( window );
                channel->init(); // not in ctor, virtual method
            }
         channelFields
        '}' { channel = 0; }
channelFields: /*null*/ | channelFields channelField
channelField:
    EQTOKEN_NAME STRING { channel->setName( $2 ); }
    | EQTOKEN_ATTRIBUTES '{' channelAttributes '}'
    | EQTOKEN_VIEWPORT viewport
        {
            if( $2[2] > 1 || $2[3] > 1 )
                channel->setPixelViewport( eq::fabric::PixelViewport
                    ( (int)$2[0], (int)$2[1], (int)$2[2], (int)$2[3] ));
            else
                channel->setViewport(eq::fabric::Viewport( $2[0], $2[1],
                                                           $2[2], $2[3]));
        }
channelAttributes: /*null*/ | channelAttributes channelAttribute
channelAttribute:
    EQTOKEN_HINT_STATISTICS IATTR
        { channel->setIAttribute( eq::server::Channel::IATTR_HINT_STATISTICS,
                                  $2 ); }
    | EQTOKEN_HINT_SENDTOKEN IATTR
        { channel->setIAttribute( eq::server::Channel::IATTR_HINT_SENDTOKEN,
                                  $2 ); }
    | EQTOKEN_DUMP_IMAGE STRING
        { channel->setSAttribute( eq::server::Channel::SATTR_DUMP_IMAGE,
                                  $2 ); }
observer: EQTOKEN_OBSERVER '{' { observer = new eq::server::Observer( config );}
            observerFields '}' { observer = 0; }
observerFields: /*null*/ | observerFields observerField
observerField:
    EQTOKEN_NAME STRING { observer->setName( $2 ); }
    | EQTOKEN_EYE_BASE FLOAT
    {
        const float eyeBase_2 = $2 * .5f;
        observer->setEyePosition( eq::fabric::EYE_LEFT,
                                  eq::fabric::Vector3f( -eyeBase_2, 0.f, 0.f ));
        observer->setEyePosition( eq::fabric::EYE_CYCLOP,
                                  eq::fabric::Vector3f::ZERO );
        observer->setEyePosition( eq::fabric::EYE_RIGHT,
                                  eq::fabric::Vector3f( eyeBase_2, 0.f, 0.f ));
    }
    | EQTOKEN_EYE_LEFT  '[' FLOAT FLOAT FLOAT ']'
    {
        observer->setEyePosition( eq::fabric::EYE_LEFT,
                                  eq::fabric::Vector3f( $3, $4, $5 ));
    }
    | EQTOKEN_EYE_CYCLOP  '[' FLOAT FLOAT FLOAT ']'
    {
        observer->setEyePosition( eq::fabric::EYE_CYCLOP,
                                  eq::fabric::Vector3f( $3, $4, $5 ));
    }
    | EQTOKEN_EYE_RIGHT  '[' FLOAT FLOAT FLOAT ']'
    {
        observer->setEyePosition( eq::fabric::EYE_RIGHT,
                                  eq::fabric::Vector3f( $3, $4, $5 ));
    }
    | EQTOKEN_FOCUS_DISTANCE FLOAT { observer->setFocusDistance( $2 ); }
    | EQTOKEN_FOCUS_MODE IATTR
        { observer->setFocusMode( eq::fabric::FocusMode( $2 )); }
    | EQTOKEN_OPENCV_CAMERA IATTR { observer->setOpenCVCamera( $2 ); }
    | EQTOKEN_VRPN_TRACKER STRING { observer->setVRPNTracker( $2 ); }

layout: EQTOKEN_LAYOUT '{' { layout = new eq::server::Layout( config ); }
            layoutFields '}' { layout = 0; }
layoutFields: /*null*/ | layoutFields layoutField
layoutField:
    EQTOKEN_NAME STRING { layout->setName( $2 ); }
    | view

view: EQTOKEN_VIEW '{' { view = new eq::server::View( layout ); }
          viewFields '}' { view = 0; }
viewFields: /*null*/ | viewFields viewField
viewField:
    EQTOKEN_ATTRIBUTES '{' viewAttributes '}'
    | EQTOKEN_NAME STRING { view->setName( $2 ); }
    | EQTOKEN_DISPLAYCLUSTER STRING /* backward compat */
      {
        view->setSAttribute( eq::server::View::SATTR_DISPLAYCLUSTER, $2 );
      }
    | EQTOKEN_MODE { view->changeMode( eq::server::View::MODE_MONO ); }
        viewMode
    | EQTOKEN_VIEWPORT viewport
        { view->setViewport( eq::fabric::Viewport( $2[0], $2[1],
                                                   $2[2], $2[3] ));}
    | wall       { view->setWall( wall ); }
    | projection { view->setProjection( projection ); }
    | EQTOKEN_OBSERVER STRING
      {
          eq::server::Observer* ob = config->find< eq::server::Observer >( $2 );
          if( ob )
              view->setObserver( ob );
          else
          {
              yyerror( "No observer of the given name" );
              YYERROR;
          }
      }
    | EQTOKEN_OBSERVER UNSIGNED
      {
          const eq::server::ObserverPath path( $2 );
          eq::server::Observer* ob = config->getObserver( path );
          if( ob )
              view->setObserver( ob );
          else
          {
              yyerror( "No observer of the given index" );
              YYERROR;
          }
      }

viewMode:
    EQTOKEN_MONO  { view->changeMode( eq::server::View::MODE_MONO ); }
    | EQTOKEN_STEREO  { view->changeMode( eq::server::View::MODE_STEREO ); }

viewAttributes: /*null*/ | viewAttributes viewAttribute
viewAttribute:
     EQTOKEN_DISPLAYCLUSTER STRING
     {
        view->setSAttribute( eq::server::View::SATTR_DISPLAYCLUSTER, $2 );
     }

canvas: EQTOKEN_CANVAS '{' { canvas = new eq::server::Canvas( config ); }
            canvasFields '}' { config->activateCanvas( canvas ); canvas = 0; }
canvasFields: /*null*/ | canvasFields canvasField
canvasField:
    EQTOKEN_NAME STRING { canvas->setName( $2 ); }
    | EQTOKEN_LAYOUT STRING
      {
          eq::server::Layout* l = config->find< eq::server::Layout >( $2 );
          if( l )
              canvas->addLayout( l );
          else
          {
              yyerror( "No layout of the given name" );
              YYERROR;
          }
      }
    | swapBarrier { canvas->setSwapBarrier( swapBarrier ); swapBarrier = 0; }
    | EQTOKEN_LAYOUT UNSIGNED
      {
          const eq::server::LayoutPath path( $2 );
          eq::server::Layout* l = config->getLayout( path );
          if( l )
              canvas->addLayout( l );
          else
          {
              yyerror( "No layout of the given index" );
              YYERROR;
          }
      }
    | EQTOKEN_LAYOUT EQTOKEN_OFF
      {
          canvas->addLayout( 0 );
      }
    | wall       { canvas->setWall( wall ); }
    | projection { canvas->setProjection( projection ); }
    | segment

segment: EQTOKEN_SEGMENT '{' { segment = new eq::server::Segment( canvas ); }
          segmentFields '}'
segmentFields: /*null*/ | segmentFields segmentField
segmentField:
    EQTOKEN_NAME STRING { segment->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
        {
            eq::server::Channel* ch = config->find< eq::server::Channel >( $2 );
            if( ch )
                segment->setChannel( ch );
            else
            {
                yyerror( "No channel of the given name" );
                YYERROR;
            }
        }
    | EQTOKEN_EYE  '['   { segment->setEyes( eq::fabric::EYE_UNDEFINED );}
        segmentEyes  ']'
    | EQTOKEN_VIEWPORT viewport
        { segment->setViewport( eq::fabric::Viewport( $2[0], $2[1],
                                                      $2[2], $2[3] ));}
    | swapBarrier { segment->setSwapBarrier( swapBarrier ); swapBarrier = 0; }
    | wall       { segment->setWall( wall ); }
    | projection { segment->setProjection( projection ); }

segmentEyes: /*null*/ | segmentEyes segumentEye
segumentEye:
    EQTOKEN_CYCLOP  { segment->enableEye( eq::fabric::EYE_CYCLOP ); }
    | EQTOKEN_LEFT  { segment->enableEye( eq::fabric::EYE_LEFT ); }
    | EQTOKEN_RIGHT { segment->enableEye( eq::fabric::EYE_RIGHT ); }

compound: EQTOKEN_COMPOUND '{'
              {
                  if( eqCompound )
                      eqCompound = new eq::server::Compound( eqCompound );
                  else
                      eqCompound = new eq::server::Compound( config );
              }
          compoundFields
          '}' { eqCompound = eqCompound->getParent(); }

compoundFields: /*null*/ | compoundFields compoundField
compoundField:
    compound
    | EQTOKEN_NAME STRING { eqCompound->setName( $2 ); }
    | EQTOKEN_CHANNEL STRING
      {
          eq::server::Channel* ch = config->find< eq::server::Channel >( $2 );
          if( ch )
              eqCompound->setChannel( ch );
          else
          {
              yyerror( "No channel of the given name" );
              YYERROR;
          }
      }
    | EQTOKEN_CHANNEL viewSegmentRef
      {
          if( !segment || !view )
          {
              yyerror( "Incomplete channel reference (view or segment missing)" );
              YYERROR;
          }
          else
          {
              eq::server::Channel* ch = config->findChannel( segment, view );
              if( ch )
                  eqCompound->setChannel( ch );
              else
              {
                  yyerror( "No channel for the given view and segment" );
                  YYERROR;
              }
          }

          canvas = 0;
          segment = 0;
          layout = 0;
          view = 0;
      }
    | EQTOKEN_TASK '['   { eqCompound->setTasks( eq::fabric::TASK_NONE ); }
        compoundTasks ']'
    | EQTOKEN_EYE  '['   { eqCompound->setEyes( eq::fabric::EYE_UNDEFINED );}
        compoundEyes  ']'
    | EQTOKEN_BUFFER '[' { flags = eq::fabric::Frame::BUFFER_NONE; }
        buffers ']' { eqCompound->setBuffers( flags ); flags = 0; }
    | EQTOKEN_VIEWPORT viewport
        { eqCompound->setViewport( eq::fabric::Viewport( $2[0], $2[1],
                                                         $2[2], $2[3] ));}
    | EQTOKEN_RANGE '[' FLOAT FLOAT ']'
        { eqCompound->setRange( eq::fabric::Range( $3, $4 )); }
    | EQTOKEN_PERIOD UNSIGNED { eqCompound->setPeriod( $2 ); }
    | EQTOKEN_PHASE  UNSIGNED { eqCompound->setPhase( $2 ); }
    | EQTOKEN_ZOOM '[' FLOAT FLOAT ']'
        { eqCompound->setZoom( eq::fabric::Zoom( $3, $4 )); }
    | EQTOKEN_PIXEL '[' UNSIGNED UNSIGNED UNSIGNED UNSIGNED ']'
        { eqCompound->setPixel( eq::fabric::Pixel( $3, $4, $5, $6 )); }
    | EQTOKEN_SUBPIXEL '[' UNSIGNED UNSIGNED ']'
        { eqCompound->setSubPixel( eq::fabric::SubPixel( $3, $4 )); }
    | wall { eqCompound->setWall( wall ); }
    | projection { eqCompound->setProjection( projection ); }
    | equalizer
    | swapBarrier { eqCompound->setSwapBarrier(swapBarrier); swapBarrier = 0; }
    | outputFrame
    | inputFrame
    | outputTiles
    | inputTiles
    | EQTOKEN_ATTRIBUTES '{' compoundAttributes '}'

viewSegmentRef:
    '(' {
            canvas = 0;
            segment = config->getSegment( eq::server::SegmentPath( 0 ));
            layout = 0;
            view = config->getView( eq::server::ViewPath( 0 ));;
        }
    viewSegmentRefFields ')'

viewSegmentRefFields : /*null*/ | viewSegmentRefFields viewSegmentRefField
viewSegmentRefField:
    EQTOKEN_CANVAS STRING
        {
            canvas = config->find< eq::server::Canvas >( $2 );
            if( !canvas )
            {
                yyerror( "Can't find canvas" );
                YYERROR;
            }
            segment = canvas->getSegment( eq::server::SegmentPath( 0 ));
        }
    | EQTOKEN_CANVAS UNSIGNED
        {
            canvas = config->getCanvas( eq::server::CanvasPath( $2 ));
            if( !canvas )
            {
                yyerror( "Can't find canvas" );
                YYERROR;
            }
            segment = canvas->getSegment( eq::server::SegmentPath( 0 ));
        }
    | EQTOKEN_SEGMENT STRING
        {
            if( canvas )
                segment = canvas->findSegment( $2 );
            else
                segment = config->find< eq::server::Segment >( $2 );
        }
    | EQTOKEN_SEGMENT UNSIGNED
        {
            if( canvas )
                segment = canvas->getSegment( eq::server::SegmentPath( $2 ));
            else
                segment = config->getSegment( eq::server::SegmentPath( $2 ));
        }
    | EQTOKEN_LAYOUT STRING
        {
            layout = config->find< eq::server::Layout >( $2 );
            if( !layout )
            {
                yyerror( "Can't find layout" );
                YYERROR;
            }
            view = layout->getView( eq::server::ViewPath( 0 ));;
        }
    | EQTOKEN_LAYOUT UNSIGNED
        {
            layout = config->getLayout( eq::server::LayoutPath( $2 ));
            if( !layout )
            {
                yyerror( "Can't find layout" );
                YYERROR;
            }
            view = layout->getView( eq::server::ViewPath( 0 ));;
        }
    | EQTOKEN_VIEW STRING
        {
            if( layout )
                view = layout->findView( $2 );
            else
                view = config->find< eq::server::View >( $2 );
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
    EQTOKEN_CLEAR      { eqCompound->enableTask( eq::fabric::TASK_CLEAR ); }
    | EQTOKEN_DRAW     { eqCompound->enableTask( eq::fabric::TASK_DRAW ); }
    | EQTOKEN_ASSEMBLE { eqCompound->enableTask( eq::fabric::TASK_ASSEMBLE );}
    | EQTOKEN_READBACK { eqCompound->enableTask( eq::fabric::TASK_READBACK );}

compoundEyes: /*null*/ | compoundEyes compoundEye
compoundEye:
    EQTOKEN_CYCLOP  { eqCompound->enableEye( eq::fabric::EYE_CYCLOP ); }
    | EQTOKEN_LEFT  { eqCompound->enableEye( eq::fabric::EYE_LEFT ); }
    | EQTOKEN_RIGHT { eqCompound->enableEye( eq::fabric::EYE_RIGHT ); }

buffers: /*null*/ | buffers buffer
buffer:
    EQTOKEN_COLOR    { flags |= eq::fabric::Frame::BUFFER_COLOR; }
    | EQTOKEN_DEPTH  { flags |= eq::fabric::Frame::BUFFER_DEPTH; }

wall: EQTOKEN_WALL '{' { wall = eq::fabric::Wall(); } wallFields '}'

wallFields:  /*null*/ | wallFields wallField
wallField:
    EQTOKEN_BOTTOM_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomLeft = eq::fabric::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_BOTTOM_RIGHT  '[' FLOAT FLOAT FLOAT ']'
        { wall.bottomRight = eq::fabric::Vector3f( $3, $4, $5 ); }
    |  EQTOKEN_TOP_LEFT  '[' FLOAT FLOAT FLOAT ']'
        { wall.topLeft = eq::fabric::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_TYPE wallType

wallType:
    EQTOKEN_FIXED { wall.type = eq::fabric::Wall::TYPE_FIXED; }
    | EQTOKEN_HMD { wall.type = eq::fabric::Wall::TYPE_HMD; }

projection: EQTOKEN_PROJECTION '{' { projection = eq::fabric::Projection(); }
                projectionFields '}'

projectionFields:  /*null*/ | projectionFields projectionField
projectionField:
    EQTOKEN_ORIGIN  '[' FLOAT FLOAT FLOAT ']'
        { projection.origin = eq::fabric::Vector3f( $3, $4, $5 ); }
    | EQTOKEN_DISTANCE FLOAT
        { projection.distance = $2; }
    | EQTOKEN_FOV  '[' FLOAT FLOAT ']'
        { projection.fov = eq::fabric::Vector2f( $3, $4 ); }
    | EQTOKEN_HPR  '[' FLOAT FLOAT FLOAT ']'
        { projection.hpr = eq::fabric::Vector3f( $3, $4, $5 ); }

equalizer: dfrEqualizer | framerateEqualizer | loadEqualizer | treeEqualizer |
           monitorEqualizer | viewEqualizer | tileEqualizer

dfrEqualizer: EQTOKEN_DFREQUALIZER '{'
    { dfrEqualizer = new eq::server::DFREqualizer; }
    dfrEqualizerFields '}'
    {
        eqCompound->addEqualizer( dfrEqualizer );
        dfrEqualizer = 0;
    }
framerateEqualizer: EQTOKEN_FRAMERATEEQUALIZER '{' '}'
    {
        eqCompound->addEqualizer( new eq::server::FramerateEqualizer );
    }
loadEqualizer: EQTOKEN_LOADEQUALIZER '{'
    { loadEqualizer = new eq::server::LoadEqualizer; }
    loadEqualizerFields '}'
    {
        eqCompound->addEqualizer( loadEqualizer );
        loadEqualizer = 0;
    }
treeEqualizer: EQTOKEN_TREEEQUALIZER '{'
    { treeEqualizer = new eq::server::TreeEqualizer; }
    treeEqualizerFields '}'
    {
        eqCompound->addEqualizer( treeEqualizer );
        treeEqualizer = 0;
    }
monitorEqualizer: EQTOKEN_MONITOREQUALIZER '{' '}'
    {
        eqCompound->addEqualizer( new eq::server::MonitorEqualizer );
    }
viewEqualizer: EQTOKEN_VIEWEQUALIZER '{' '}'
    {
        eqCompound->addEqualizer( new eq::server::ViewEqualizer );
    }
tileEqualizer: EQTOKEN_TILEEQUALIZER
    '{' { tileEqualizer = new eq::server::TileEqualizer; }
    tileEqualizerFields '}'
    {
        eqCompound->addEqualizer( tileEqualizer );
        tileEqualizer = 0;
    }

dfrEqualizerFields: /* null */ | dfrEqualizerFields dfrEqualizerField
dfrEqualizerField:
    EQTOKEN_DAMPING FLOAT      { dfrEqualizer->setDamping( $2 ); }
    | EQTOKEN_FRAMERATE FLOAT  { dfrEqualizer->setFrameRate( $2 ); }

loadEqualizerFields: /* null */ | loadEqualizerFields loadEqualizerField
loadEqualizerField:
    EQTOKEN_DAMPING FLOAT            { loadEqualizer->setDamping( $2 ); }
    | EQTOKEN_BOUNDARY '[' UNSIGNED UNSIGNED ']'
                 { loadEqualizer->setBoundary( eq::fabric::Vector2i( $3, $4 )); }
    | EQTOKEN_ASSEMBLE_ONLY_LIMIT FLOAT
                           { loadEqualizer->setAssembleOnlyLimit( $2 ); }
    | EQTOKEN_BOUNDARY FLOAT        { loadEqualizer->setBoundary( $2 ); }
    | EQTOKEN_MODE loadEqualizerMode    { loadEqualizer->setMode( $2 ); }
    | EQTOKEN_RESISTANCE '[' UNSIGNED UNSIGNED ']'
        { loadEqualizer->setResistance( eq::fabric::Vector2i( $3, $4 )); }
    | EQTOKEN_RESISTANCE FLOAT  { loadEqualizer->setResistance( $2 ); }

loadEqualizerMode:
    EQTOKEN_2D           { $$ = eq::server::LoadEqualizer::MODE_2D; }
    | EQTOKEN_DB         { $$ = eq::server::LoadEqualizer::MODE_DB; }
    | EQTOKEN_HORIZONTAL { $$ = eq::server::LoadEqualizer::MODE_HORIZONTAL; }
    | EQTOKEN_VERTICAL   { $$ = eq::server::LoadEqualizer::MODE_VERTICAL; }

treeEqualizerFields: /* null */ | treeEqualizerFields treeEqualizerField
treeEqualizerField:
    EQTOKEN_DAMPING FLOAT            { treeEqualizer->setDamping( $2 ); }
    | EQTOKEN_BOUNDARY '[' UNSIGNED UNSIGNED ']'
                 { treeEqualizer->setBoundary( eq::fabric::Vector2i( $3, $4 )); }
    | EQTOKEN_BOUNDARY FLOAT        { treeEqualizer->setBoundary( $2 ); }
    | EQTOKEN_MODE treeEqualizerMode    { treeEqualizer->setMode( $2 ); }
    | EQTOKEN_RESISTANCE '[' UNSIGNED UNSIGNED ']'
        { treeEqualizer->setResistance( eq::fabric::Vector2i( $3, $4 )); }
    | EQTOKEN_RESISTANCE FLOAT  { treeEqualizer->setResistance( $2 ); }

treeEqualizerMode:
    EQTOKEN_2D           { $$ = eq::server::TreeEqualizer::MODE_2D; }
    | EQTOKEN_DB         { $$ = eq::server::TreeEqualizer::MODE_DB; }
    | EQTOKEN_HORIZONTAL { $$ = eq::server::TreeEqualizer::MODE_HORIZONTAL; }
    | EQTOKEN_VERTICAL   { $$ = eq::server::TreeEqualizer::MODE_VERTICAL; }

tileEqualizerFields: /* null */ | tileEqualizerFields tileEqualizerField
tileEqualizerField:
    EQTOKEN_NAME STRING                   { tileEqualizer->setName( $2 ); }
    | EQTOKEN_SIZE '[' UNSIGNED UNSIGNED ']'
                   { tileEqualizer->setTileSize( eq::fabric::Vector2i( $3, $4 )); }

swapBarrier:
    EQTOKEN_SWAPBARRIER '{' { swapBarrier = new eq::server::SwapBarrier; }
    swapBarrierFields '}'

swapBarrierFields: /*null*/ | swapBarrierFields swapBarrierField
swapBarrierField: EQTOKEN_NAME STRING { swapBarrier->setName( $2 ); }
    | EQTOKEN_NVGROUP IATTR { swapBarrier->setNVSwapGroup( $2 ); }
    | EQTOKEN_NVBARRIER IATTR { swapBarrier->setNVSwapBarrier( $2 ); }



outputFrame: EQTOKEN_OUTPUTFRAME '{' { frame = new eq::server::Frame; }
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
        { frame->setViewport(eq::fabric::Viewport( $2[0], $2[1],
                                                   $2[2], $2[3])); }
    | EQTOKEN_BUFFER '[' { flags = eq::fabric::Frame::BUFFER_NONE; }
        buffers ']' { frame->setBuffers( flags ); flags = 0; }
    | EQTOKEN_ZOOM '[' FLOAT FLOAT ']'
        { frame->setNativeZoom( eq::fabric::Zoom( $3, $4 )); }

frameType:
    EQTOKEN_TEXTURE { frame->setType( eq::fabric::Frame::TYPE_TEXTURE ); }
    | EQTOKEN_MEMORY { frame->setType( eq::fabric::Frame::TYPE_MEMORY ); }

outputTiles: EQTOKEN_OUTPUTTILES '{' { tileQueue = new eq::server::TileQueue; }
    tileQueueFields '}'
        {
            eqCompound->addOutputTileQueue( tileQueue );
            tileQueue = 0;
        }
inputTiles: EQTOKEN_INPUTTILES '{' { tileQueue = new eq::server::TileQueue; }
    tileQueueFields '}'
        {
            eqCompound->addInputTileQueue( tileQueue );
            tileQueue = 0;
        }
tileQueueFields: /*null*/ | tileQueueFields tileQueueField
tileQueueField:
    EQTOKEN_NAME STRING { tileQueue->setName( $2 ); }
    | EQTOKEN_SIZE '[' UNSIGNED UNSIGNED ']'
        { tileQueue->setTileSize( eq::fabric::Vector2i( $3, $4 )); }

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
        { LBWARN << "ignoring removed attribute update_FOV" << std::endl; }

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
    EQTOKEN_ON           { $$ = eq::fabric::ON; }
    | EQTOKEN_OFF        { $$ = eq::fabric::OFF; }
    | EQTOKEN_AUTO       { $$ = eq::fabric::AUTO; }
    | EQTOKEN_FASTEST    { $$ = eq::fabric::FASTEST; }
    | EQTOKEN_HORIZONTAL { $$ = eq::fabric::HORIZONTAL; }
    | EQTOKEN_NICEST     { $$ = eq::fabric::NICEST; }
    | EQTOKEN_QUAD       { $$ = eq::fabric::QUAD; }
    | EQTOKEN_ANAGLYPH   { $$ = eq::fabric::ANAGLYPH; }
    | EQTOKEN_PASSIVE    { $$ = eq::fabric::PASSIVE; }
    | EQTOKEN_VERTICAL   { $$ = eq::fabric::VERTICAL; }
    | EQTOKEN_WINDOW     { $$ = eq::fabric::WINDOW; }
    | EQTOKEN_FBO        { $$ = eq::fabric::FBO; }
    | EQTOKEN_PBUFFER    { $$ = eq::fabric::PBUFFER; }
    | EQTOKEN_ASYNC      { $$ = eq::fabric::ASYNC; }
    | EQTOKEN_DRAW_SYNC  { $$ = eq::fabric::DRAW_SYNC; }
    | EQTOKEN_LOCAL_SYNC { $$ = eq::fabric::LOCAL_SYNC; }
    | EQTOKEN_RGBA16F    { $$ = eq::fabric::RGBA16F; }
    | EQTOKEN_RGBA32F    { $$ = eq::fabric::RGBA32F; }
    | EQTOKEN_FIXED      { $$ = eq::fabric::FIXED; }
    | EQTOKEN_RELATIVE_TO_ORIGIN   { $$ = eq::fabric::RELATIVE_TO_ORIGIN; }
    | EQTOKEN_RELATIVE_TO_OBSERVER { $$ = eq::fabric::RELATIVE_TO_OBSERVER; }
    | INTEGER            { $$ = $1; }
    | EQTOKEN_CORE INTEGER { $$ = eq::fabric::CORE + $2; }
    | EQTOKEN_SOCKET INTEGER  { $$ = eq::fabric::SOCKET  + $2; }

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

namespace eq
{
namespace server
{

//---------------------------------------------------------------------------
// loader
//---------------------------------------------------------------------------
ServerPtr Loader::loadFile( const std::string& filename )
{
    yyin       = fopen( filename.c_str(), "r" );
    yyinString = 0;

    if( !yyin )
    {
        LBERROR << "Can't open config file " << filename << std::endl;
        return 0;
    }

    loader::filename = filename;
    _parse();
    loader::filename.clear();

    fclose( yyin );

    eq::server::ServerPtr server = loader::server;
    loader::server = 0;
    return server;
}

void Loader::_parseString( const char* data )
{
    yyin       = 0;
    yyinString = data;
    _parse();
}

void Loader::_parse()
{
    LBASSERTINFO( !eq::loader::loader, "Config file loader is not reentrant" );
    eq::loader::loader = this;

    loader::server = 0;
    config = 0;
    yylineno = 0;

    const std::string oldLocale = setlocale( LC_NUMERIC, "C" );
    const bool error = ( eqLoader_parse() != 0 );
    setlocale( LC_NUMERIC, oldLocale.c_str( ));

    if( error )
        loader::server = 0;

    eq::loader::loader = 0;
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
