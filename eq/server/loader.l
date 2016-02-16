
%{
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "compound.h"
#include "equalizers/loadEqualizer.h"
#include "equalizers/treeEqualizer.h"
#include <co/connectionType.h>

#include "parser.hpp"

#include <lunchbox/os.h>
#include <stdlib.h>

#define yyerror eqLoader_error
#define yylineno eqLoader_lineno
#define yylex eqLoader_lex

#ifdef WIN32_API
#  include <io.h>
#  define isatty _isatty
#endif
#pragma warning(disable: 4267)
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wnull-conversion"
#pragma clang diagnostic ignored "-Wdeprecated"
#pragma clang diagnostic ignored "-Wdeprecated-register"

int yylineno = 0;
const char* yyinString = 0;

void yyerror( const char *errmsg );
void yyerror( const char *errmsg )
{
    LBERROR << "Parse error: '" << errmsg << "', line " << yylineno+1
            << " at '" << yytext << "' from" << lunchbox::backtrace <<std::endl;
    YY_FLUSH_BUFFER;
}

/* Override YY_INPUT to read from a char* if specified. YY_INPUT_FILE is C&P
 * from the generated YY_INPUT */
#define YY_INPUT_FILE( buf, result, max_size )              \
    if ( ((result = fread( buf, 1, max_size, yyin )) == 0)  \
           && ferror( yyin ) )                              \
        YY_FATAL_ERROR( "input in flex scanner failed" );

#define YY_INPUT( buf, result, max_size )                      \
    if( yyinString )                                           \
    {                                                          \
        result = LB_MIN( (size_t)(max_size), strlen( yyinString )); \
        if( result )                                           \
            memcpy( buf, yyinString, result );                 \
        yyinString += result;                                  \
    }                                                          \
    else                                                       \
    {                                                          \
        YY_INPUT_FILE( buf, result, max_size );                \
    }
%}

%option noyywrap
%option nounput

%%

#Equalizer                       { return EQTOKEN_HEADER; }
ascii                            { return EQTOKEN_ASCII; }
global                           { return EQTOKEN_GLOBAL; }
EQ_CONNECTION_SATTR_HOSTNAME     { return EQTOKEN_CONNECTION_SATTR_HOSTNAME; }
  /* Deprecated */
EQ_CONNECTION_SATTR_LAUNCH_COMMAND { return EQTOKEN_NODE_SATTR_LAUNCH_COMMAND; }
  /* Deprecated */
EQ_CONNECTION_CATTR_LAUNCH_COMMAND_QUOTE { return EQTOKEN_NODE_CATTR_LAUNCH_COMMAND_QUOTE; }
  /* Deprecated */
EQ_CONNECTION_IATTR_LAUNCH_TIMEOUT { return EQTOKEN_NODE_IATTR_LAUNCH_TIMEOUT; }
EQ_CONNECTION_IATTR_TYPE         { return EQTOKEN_CONNECTION_IATTR_TYPE; }
  /* Deprecated */
EQ_CONNECTION_IATTR_TCPIP_PORT   { return EQTOKEN_CONNECTION_IATTR_PORT; }
EQ_CONNECTION_IATTR_PORT         { return EQTOKEN_CONNECTION_IATTR_PORT; }
  /* Deprecated */
EQ_CONNECTION_SATTR_PIPE_FILENAME { return EQTOKEN_CONNECTION_SATTR_FILENAME; }
EQ_CONNECTION_SATTR_FILENAME     { return EQTOKEN_CONNECTION_SATTR_FILENAME; }
EQ_CONNECTION_IATTR_BANDWIDTH    { return EQTOKEN_CONNECTION_IATTR_BANDWIDTH; }
EQ_CONFIG_FATTR_EYE_BASE         { return EQTOKEN_CONFIG_FATTR_EYE_BASE; }
EQ_CONFIG_IATTR_ROBUSTNESS       { return EQTOKEN_CONFIG_IATTR_ROBUSTNESS; }
EQ_NODE_SATTR_LAUNCH_COMMAND     { return EQTOKEN_NODE_SATTR_LAUNCH_COMMAND; }
EQ_NODE_CATTR_LAUNCH_COMMAND_QUOTE { return EQTOKEN_NODE_CATTR_LAUNCH_COMMAND_QUOTE; }
EQ_NODE_IATTR_THREAD_MODEL       { return EQTOKEN_NODE_IATTR_THREAD_MODEL; }
EQ_NODE_IATTR_HINT_AFFINITY      { return EQTOKEN_NODE_IATTR_HINT_AFFINITY; }
EQ_NODE_IATTR_LAUNCH_TIMEOUT     { return EQTOKEN_NODE_IATTR_LAUNCH_TIMEOUT; }
EQ_NODE_IATTR_HINT_STATISTICS    { return EQTOKEN_NODE_IATTR_HINT_STATISTICS; }
EQ_PIPE_IATTR_HINT_THREAD        { return EQTOKEN_PIPE_IATTR_HINT_THREAD; }
EQ_PIPE_IATTR_HINT_AFFINITY      { return EQTOKEN_PIPE_IATTR_HINT_AFFINITY; }
EQ_PIPE_IATTR_HINT_CUDA_GL_INTEROP { return EQTOKEN_PIPE_IATTR_HINT_CUDA_GL_INTEROP; }
EQ_VIEW_SATTR_DISPLAYCLUSTER      { return EQTOKEN_VIEW_SATTR_DISPLAYCLUSTER; }
EQ_WINDOW_IATTR_HINT_CORE_PROFILE { return EQTOKEN_WINDOW_IATTR_HINT_CORE_PROFILE; }
EQ_WINDOW_IATTR_HINT_OPENGL_MAJOR { return EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MAJOR; }
EQ_WINDOW_IATTR_HINT_OPENGL_MINOR { return EQTOKEN_WINDOW_IATTR_HINT_OPENGL_MINOR; }
EQ_WINDOW_IATTR_HINT_STEREO      { return EQTOKEN_WINDOW_IATTR_HINT_STEREO; }
EQ_WINDOW_IATTR_HINT_DOUBLEBUFFER { return EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER; }
EQ_WINDOW_IATTR_HINT_FULLSCREEN  { return EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN;}
EQ_WINDOW_IATTR_HINT_DECORATION  { return EQTOKEN_WINDOW_IATTR_HINT_DECORATION;}
EQ_WINDOW_IATTR_HINT_SWAPSYNC    { return EQTOKEN_WINDOW_IATTR_HINT_SWAPSYNC; }
EQ_WINDOW_IATTR_HINT_DRAWABLE    { return EQTOKEN_WINDOW_IATTR_HINT_DRAWABLE; }
EQ_WINDOW_IATTR_HINT_STATISTICS { return EQTOKEN_WINDOW_IATTR_HINT_STATISTICS; }
EQ_WINDOW_IATTR_HINT_SCREENSAVER {return EQTOKEN_WINDOW_IATTR_HINT_SCREENSAVER;}
EQ_WINDOW_IATTR_HINT_GRAB_POINTER {return EQTOKEN_WINDOW_IATTR_HINT_GRAB_POINTER;}
EQ_WINDOW_IATTR_PLANES_COLOR     { return EQTOKEN_WINDOW_IATTR_PLANES_COLOR; }
EQ_WINDOW_IATTR_PLANES_ALPHA     { return EQTOKEN_WINDOW_IATTR_PLANES_ALPHA; }
EQ_WINDOW_IATTR_PLANES_DEPTH     { return EQTOKEN_WINDOW_IATTR_PLANES_DEPTH; }
EQ_WINDOW_IATTR_PLANES_STENCIL   { return EQTOKEN_WINDOW_IATTR_PLANES_STENCIL; }
EQ_WINDOW_IATTR_PLANES_ACCUM     { return EQTOKEN_WINDOW_IATTR_PLANES_ACCUM; }
EQ_WINDOW_IATTR_PLANES_ACCUM_ALPHA { return EQTOKEN_WINDOW_IATTR_PLANES_ACCUM_ALPHA; }
EQ_WINDOW_IATTR_PLANES_SAMPLES   { return EQTOKEN_WINDOW_IATTR_PLANES_SAMPLES; }
EQ_CHANNEL_IATTR_HINT_STATISTICS { return EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS; }
EQ_CHANNEL_IATTR_HINT_SENDTOKEN  { return EQTOKEN_CHANNEL_IATTR_HINT_SENDTOKEN; }
EQ_CHANNEL_SATTR_DUMP_IMAGE      { return EQTOKEN_CHANNEL_SATTR_DUMP_IMAGE; }
EQ_COMPOUND_IATTR_STEREO_MODE    { return EQTOKEN_COMPOUND_IATTR_STEREO_MODE; }
EQ_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK  { return EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK; }
EQ_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK { return EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK; }
EQ_COMPOUND_IATTR_UPDATE_FOV    { return EQTOKEN_COMPOUND_IATTR_UPDATE_FOV; }
server                          { return EQTOKEN_SERVER; }
config                          { return EQTOKEN_CONFIG; }
appNode                         { return EQTOKEN_APPNODE; }
node                            { return EQTOKEN_NODE; }
pipe                            { return EQTOKEN_PIPE; }
window                          { return EQTOKEN_WINDOW; }
attributes                      { return EQTOKEN_ATTRIBUTES; }
hint_decoration                 { return EQTOKEN_HINT_DECORATION; }
hint_doublebuffer               { return EQTOKEN_HINT_DOUBLEBUFFER; }
hint_fullscreen                 { return EQTOKEN_HINT_FULLSCREEN; }
hint_statistics                 { return EQTOKEN_HINT_STATISTICS; }
hint_sendtoken                  { return EQTOKEN_HINT_SENDTOKEN; }
hint_core_profile               { return EQTOKEN_HINT_CORE_PROFILE; }
hint_opengl_major               { return EQTOKEN_HINT_OPENGL_MAJOR; }
hint_opengl_minor               { return EQTOKEN_HINT_OPENGL_MINOR; }
hint_stereo                     { return EQTOKEN_HINT_STEREO; }
hint_swapsync                   { return EQTOKEN_HINT_SWAPSYNC; }
hint_drawable                   { return EQTOKEN_HINT_DRAWABLE; }
hint_thread                     { return EQTOKEN_HINT_THREAD; }
hint_affinity                   { return EQTOKEN_HINT_AFFINITY; }
hint_cuda_GL_interop            { return EQTOKEN_HINT_CUDA_GL_INTEROP; }
hint_screensaver                { return EQTOKEN_HINT_SCREENSAVER; }
hint_grab_pointer               { return EQTOKEN_HINT_GRAB_POINTER; }
planes_alpha                    { return EQTOKEN_PLANES_ALPHA; }
planes_color                    { return EQTOKEN_PLANES_COLOR; }
planes_depth                    { return EQTOKEN_PLANES_DEPTH; }
planes_stencil                  { return EQTOKEN_PLANES_STENCIL; }
planes_accum                    { return EQTOKEN_PLANES_ACCUM; }
planes_accum_alpha              { return EQTOKEN_PLANES_ACCUM_ALPHA; }
planes_samples                  { return EQTOKEN_PLANES_SAMPLES; }
ON                              { return EQTOKEN_ON; }
OFF                             { return EQTOKEN_OFF; }
AUTO                            { return EQTOKEN_AUTO; }
CORE                            { return EQTOKEN_CORE; }
SOCKET                          { return EQTOKEN_SOCKET; }
FASTEST                         { return EQTOKEN_FASTEST; }
NICEST                          { return EQTOKEN_NICEST; }
QUAD                            { return EQTOKEN_QUAD; }
ANAGLYPH                        { return EQTOKEN_ANAGLYPH; }
anaglyph                        { return EQTOKEN_ANAGLYPH; }
PASSIVE                         { return EQTOKEN_PASSIVE; }
RED                             { return EQTOKEN_RED; }
GREEN                           { return EQTOKEN_GREEN; }
BLUE                            { return EQTOKEN_BLUE; }
HORIZONTAL                      { return EQTOKEN_HORIZONTAL; }
VERTICAL                        { return EQTOKEN_VERTICAL; }
framerate                       { return EQTOKEN_FRAMERATE; }
channel                         { return EQTOKEN_CHANNEL; }
observer                        { return EQTOKEN_OBSERVER; }
layout                          { return EQTOKEN_LAYOUT; }
view                            { return EQTOKEN_VIEW; }
canvas                          { return EQTOKEN_CANVAS; }
segment                         { return EQTOKEN_SEGMENT; }
compound                        { return EQTOKEN_COMPOUND; }
DFR_equalizer                   { return EQTOKEN_DFREQUALIZER; }
framerate_equalizer             { return EQTOKEN_FRAMERATEEQUALIZER; }
load_equalizer                  { return EQTOKEN_LOADEQUALIZER; }
tree_equalizer                  { return EQTOKEN_TREEEQUALIZER; }
monitor_equalizer               { return EQTOKEN_MONITOREQUALIZER; }
view_equalizer                  { return EQTOKEN_VIEWEQUALIZER; }
tile_equalizer                  { return EQTOKEN_TILEEQUALIZER; }
damping                         { return EQTOKEN_DAMPING; }
connection                      { return EQTOKEN_CONNECTION; }
name                            { return EQTOKEN_NAME; }
type                            { return EQTOKEN_TYPE; }
TCPIP                           { return EQTOKEN_TCPIP; }
SDP                             { return EQTOKEN_SDP; }
PIPE                            { return EQTOKEN_PIPE; }
RSP                             { return EQTOKEN_RSP; }
RDMA                            { return EQTOKEN_RDMA; }
UDT                             { return EQTOKEN_UDT; }
texture                         { return EQTOKEN_TEXTURE; }
memory                          { return EQTOKEN_MEMORY; }
fixed                           { return EQTOKEN_FIXED; }
relative_to_observer            { return EQTOKEN_RELATIVE_TO_OBSERVER; }
relative_to_origin              { return EQTOKEN_RELATIVE_TO_ORIGIN; }
HMD                             { return EQTOKEN_HMD; }
host                            { return EQTOKEN_HOST; }
hostname                        { return EQTOKEN_HOSTNAME; }
interface                       { return EQTOKEN_INTERFACE; }
launch_command                  { return EQTOKEN_LAUNCH_COMMAND; }
launch_command_quote            { return EQTOKEN_LAUNCH_COMMAND_QUOTE; }
launch_timeout                  { return EQTOKEN_LAUNCH_TIMEOUT; }
  /* Deprecated */
TCPIP_port                      { return EQTOKEN_PORT; }
port                            { return EQTOKEN_PORT; }
  /* Deprecated */
PIPE_filename                   { return EQTOKEN_FILENAME; }
filename                        { return EQTOKEN_FILENAME; }
task                            { return EQTOKEN_TASK; }
eye                             { return EQTOKEN_EYE; }
eye_base                        { return EQTOKEN_EYE_BASE; }
eye_left                        { return EQTOKEN_EYE_LEFT; }
eye_cyclop                      { return EQTOKEN_EYE_CYCLOP; }
eye_right                       { return EQTOKEN_EYE_RIGHT; }
focus_distance                  { return EQTOKEN_FOCUS_DISTANCE; }
focus_mode                      { return EQTOKEN_FOCUS_MODE; }
opencv_camera                   { return EQTOKEN_OPENCV_CAMERA; }
vrpn_tracker                    { return EQTOKEN_VRPN_TRACKER; }
robustness                      { return EQTOKEN_ROBUSTNESS; }
buffer                          { return EQTOKEN_BUFFER; }
CLEAR                           { return EQTOKEN_CLEAR; }
DRAW                            { return EQTOKEN_DRAW; }
CYCLOP                          { return EQTOKEN_CYCLOP; }
LEFT                            { return EQTOKEN_LEFT; }
RIGHT                           { return EQTOKEN_RIGHT; }
ASSEMBLE                        { return EQTOKEN_ASSEMBLE; }
READBACK                        { return EQTOKEN_READBACK; }
COLOR                           { return EQTOKEN_COLOR; }
DEPTH                           { return EQTOKEN_DEPTH; }
viewport                        { return EQTOKEN_VIEWPORT; }
range                           { return EQTOKEN_RANGE; }
period                          { return EQTOKEN_PERIOD; }
phase                           { return EQTOKEN_PHASE; }
pixel                           { return EQTOKEN_PIXEL; }
subpixel                        { return EQTOKEN_SUBPIXEL; }
bandwidth                       { return EQTOKEN_BANDWIDTH; }
device                          { return EQTOKEN_DEVICE; }
wall                            { return EQTOKEN_WALL; }
bottom_left                     { return EQTOKEN_BOTTOM_LEFT; }
bottom_right                    { return EQTOKEN_BOTTOM_RIGHT; }
top_left                        { return EQTOKEN_TOP_LEFT; }
projection                      { return EQTOKEN_PROJECTION; }
origin                          { return EQTOKEN_ORIGIN; }
distance                        { return EQTOKEN_DISTANCE; }
fov                             { return EQTOKEN_FOV; }
hpr                             { return EQTOKEN_HPR; }
latency                         { return EQTOKEN_LATENCY; }
swapbarrier                     { return EQTOKEN_SWAPBARRIER; }
NV_group                        { return EQTOKEN_NVGROUP;}
NV_barrier                      { return EQTOKEN_NVBARRIER;}
outputframe                     { return EQTOKEN_OUTPUTFRAME; }
inputframe                      { return EQTOKEN_INPUTFRAME; }
outputtiles                     { return EQTOKEN_OUTPUTTILES; }
inputtiles                      { return EQTOKEN_INPUTTILES; }
stereo_mode                     { return EQTOKEN_STEREO_MODE; }
stereo_anaglyph_left_mask       { return EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK; }
stereo_anaglyph_right_mask      { return EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK; }
update_FOV                      { return EQTOKEN_UPDATE_FOV; }
FBO                             { return EQTOKEN_FBO; }
RGBA16F                         { return EQTOKEN_RGBA16F; }
RGBA32F                         { return EQTOKEN_RGBA32F; }
pbuffer                         { return EQTOKEN_PBUFFER; }
thread_model                    { return EQTOKEN_THREAD_MODEL; }
ASYNC                           { return EQTOKEN_ASYNC; }
async                           { return EQTOKEN_ASYNC; }
DRAW_SYNC                       { return EQTOKEN_DRAW_SYNC; }
draw_sync                       { return EQTOKEN_DRAW_SYNC; }
LOCAL_SYNC                      { return EQTOKEN_LOCAL_SYNC; }
local_sync                      { return EQTOKEN_LOCAL_SYNC; }
mode                            { return EQTOKEN_MODE; }
boundary                        { return EQTOKEN_BOUNDARY; }
resistance                      { return EQTOKEN_RESISTANCE; }
2D                              { return EQTOKEN_2D; }
assemble_only_limit             { return EQTOKEN_ASSEMBLE_ONLY_LIMIT; }
DB                              { return EQTOKEN_DB; }
zoom                            { return EQTOKEN_ZOOM; }
MONO                            { return EQTOKEN_MONO; }
STEREO                          { return EQTOKEN_STEREO; }
size                            { return EQTOKEN_SIZE; }
DisplayCluster                  { return EQTOKEN_DISPLAYCLUSTER; }
dump_image                      { return EQTOKEN_DUMP_IMAGE; }

[+-]?[0-9]+[\.][0-9]*           { return EQTOKEN_FLOAT; }
[+-]?[0-9]*[\.][0-9]+           { return EQTOKEN_FLOAT; }
[+]?[0-9]+                      { return EQTOKEN_UNSIGNED; }
[+-]?[0-9]+                     { return EQTOKEN_INTEGER; }
\"[-\\\., _%&@<>:a-zA-Z0-9\! \/]*\"  { return EQTOKEN_STRING; }
'.'                             { return EQTOKEN_CHARACTER; }

[\(\){}\[\]]                    { return *yytext; }
[\n]                            { ++yylineno; }
[ \t]                           { ; }
[/][/]|[#]                      { while( yyinput() != '\n' ) {} ++yylineno; }

%%
