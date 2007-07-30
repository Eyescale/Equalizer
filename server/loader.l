
%{
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"
#include "parser.hpp"
#include <stdlib.h>

#define yyerror eqLoader_error
#define yylineno eqLoader_lineno
#define yylex eqLoader_lex

#ifdef WIN32
#  include <io.h>
#  define isatty _isatty
#endif

void yyerror(char *);
int yylineno = 0;
const char* yyinString = 0;

/* Override YY_INPUT to read from a char* if specified. YY_INPUT_FILE is C&P 
 * from the generated YY_INPUT */
#define YY_INPUT_FILE( buf, result, max_size )              \
    if ( ((result = fread( buf, 1, max_size, yyin )) == 0)  \
           && ferror( yyin ) )                              \
        YY_FATAL_ERROR( "input in flex scanner failed" );

#define YY_INPUT( buf, result, max_size )                   \
    if( yyinString )                                        \
    {                                                       \
        result = MIN( max_size, (int)strlen( yyinString )); \
        if( result )                                        \
            memcpy( buf, yyinString, result );              \
        yyinString += result;                               \
    }                                                       \
    else                                                    \
    {                                                       \
        YY_INPUT_FILE( buf, result, max_size );             \
    }
%}

%option noyywrap
%option nounput

%%

global                           { return EQTOKEN_GLOBAL; }
EQ_CONNECTION_SATTR_HOSTNAME     { return EQTOKEN_CONNECTION_SATTR_HOSTNAME; }
EQ_CONNECTION_SATTR_LAUNCH_COMMAND { return EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND; }
EQ_CONNECTION_IATTR_TYPE         { return EQTOKEN_CONNECTION_IATTR_TYPE; }
EQ_CONNECTION_IATTR_TCPIP_PORT   { return EQTOKEN_CONNECTION_IATTR_TCPIP_PORT; }
EQ_CONNECTION_IATTR_LAUNCH_TIMEOUT { return EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT; }
EQ_CONFIG_FATTR_EYE_BASE         { return EQTOKEN_CONFIG_FATTR_EYE_BASE; }
EQ_PIPE_IATTR_HINT_THREAD        { return EQTOKEN_PIPE_IATTR_HINT_THREAD; }
EQ_WINDOW_IATTR_HINT_STEREO      { return EQTOKEN_WINDOW_IATTR_HINT_STEREO; }
EQ_WINDOW_IATTR_HINT_DOUBLEBUFFER { return EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER; }
EQ_WINDOW_IATTR_HINT_FULLSCREEN  { return EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN;}
EQ_WINDOW_IATTR_HINT_DECORATION  { return EQTOKEN_WINDOW_IATTR_HINT_DECORATION;}
EQ_WINDOW_IATTR_PLANES_COLOR     { return EQTOKEN_WINDOW_IATTR_PLANES_COLOR; }
EQ_WINDOW_IATTR_PLANES_ALPHA     { return EQTOKEN_WINDOW_IATTR_PLANES_ALPHA; }
EQ_WINDOW_IATTR_PLANES_DEPTH     { return EQTOKEN_WINDOW_IATTR_PLANES_DEPTH; }
EQ_WINDOW_IATTR_PLANES_STENCIL   { return EQTOKEN_WINDOW_IATTR_PLANES_STENCIL; }
EQ_CHANNEL_IATTR_HINT_STATISTICS { return EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS; }
EQ_COMPOUND_IATTR_STEREO_MODE    { return EQTOKEN_COMPOUND_IATTR_STEREO_MODE; } 
EQ_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK  { return EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK; }
EQ_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK { return EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK; }
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
hint_stereo                     { return EQTOKEN_HINT_STEREO; }
hint_thread                     { return EQTOKEN_HINT_THREAD; }
planes_alpha                    { return EQTOKEN_PLANES_ALPHA; }
planes_color                    { return EQTOKEN_PLANES_COLOR; }
planes_depth                    { return EQTOKEN_PLANES_DEPTH; }
planes_stencil                  { return EQTOKEN_PLANES_STENCIL; }
ON                              { return EQTOKEN_ON; }
OFF                             { return EQTOKEN_OFF; }
AUTO                            { return EQTOKEN_AUTO; }
FASTEST                         { return EQTOKEN_FASTEST; }
NICEST                          { return EQTOKEN_NICEST; }
QUAD                            { return EQTOKEN_QUAD; }
ANAGLYPH                        { return EQTOKEN_ANAGLYPH; }
RED                             { return EQTOKEN_RED; }
GREEN                           { return EQTOKEN_GREEN; }
BLUE                            { return EQTOKEN_BLUE; }
channel                         { return EQTOKEN_CHANNEL; }
compound                        { return EQTOKEN_COMPOUND; }
connection                      { return EQTOKEN_CONNECTION; }
name                            { return EQTOKEN_NAME; }
type                            { return EQTOKEN_TYPE; }
TCPIP                           { return EQTOKEN_TCPIP; }
SDP                             { return EQTOKEN_SDP; }
hostname                        { return EQTOKEN_HOSTNAME; }
command                         { return EQTOKEN_COMMAND; }
timeout                         { return EQTOKEN_TIMEOUT; }
TCPIP_port                      { return EQTOKEN_TCPIP_PORT; }
task                            { return EQTOKEN_TASK; }
eye                             { return EQTOKEN_EYE; }
eyeBase                         { return EQTOKEN_EYE_BASE; }
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
port                            { return EQTOKEN_PORT; }
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
outputframe                     { return EQTOKEN_OUTPUTFRAME; }
inputframe                      { return EQTOKEN_INPUTFRAME; }
stereo_mode                     { return EQTOKEN_STEREO_MODE; }
stereo_anaglyph_left_mask       { return EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK; }
stereo_anaglyph_right_mask      { return EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK; }
[+-]?[0-9]+[\.][0-9]*           { return EQTOKEN_FLOAT; }
[+-]?[0-9]*[\.][0-9]+           { return EQTOKEN_FLOAT; }
[+]?[0-9]+                      { return EQTOKEN_UNSIGNED; }
[+-]?[0-9]+                     { return EQTOKEN_INTEGER; }
\"[-\., _%<>:a-zA-Z0-9]*\"      { return EQTOKEN_STRING; }

[{}\[\]]                        { return *yytext; }
[\n]                            { ++yylineno; }
[ \t]                           { ; }
[/][/]|[#]                      { while( yyinput() != '\n' ); ++yylineno; }

%%
