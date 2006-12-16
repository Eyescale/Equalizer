
%{
#include "compound.h"
#include "parser.hpp"
#include <stdlib.h>

#define yyerror eqLoader_error
#define yylineno eqLoader_lineno
#define yylex eqLoader_lex

void yyerror(char *);
int yylineno = 0;
%}
%option noyywrap

%%

global                           { return EQTOKEN_GLOBAL; }
EQ_CONNECTION_SATTR_HOSTNAME     { return EQTOKEN_CONNECTION_SATTR_HOSTNAME; }
EQ_CONNECTION_SATTR_LAUNCH_COMMAND { return EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND; }
EQ_CONNECTION_IATTR_TYPE         { return EQTOKEN_CONNECTION_IATTR_TYPE; }
EQ_CONNECTION_IATTR_TCPIP_PORT   { return EQTOKEN_CONNECTION_IATTR_TCPIP_PORT; }
EQ_CONNECTION_IATTR_LAUNCH_TIMEOUT { return EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT; }
EQ_CONFIG_FATTR_EYE_BASE          { return EQTOKEN_CONFIG_FATTR_EYE_BASE; }
EQ_WINDOW_IATTR_HINT_STEREO       { return EQTOKEN_WINDOW_IATTR_HINT_STEREO; }
EQ_WINDOW_IATTR_HINT_DOUBLEBUFFER { return EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER; }
EQ_WINDOW_IATTR_HINT_FULLSCREEN   { return EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN; }
EQ_WINDOW_IATTR_HINT_DECORATION   { return EQTOKEN_WINDOW_IATTR_HINT_DECORATION; }
EQ_WINDOW_IATTR_PLANES_COLOR    { return EQTOKEN_WINDOW_IATTR_PLANES_COLOR; }
EQ_WINDOW_IATTR_PLANES_ALPHA    { return EQTOKEN_WINDOW_IATTR_PLANES_ALPHA; }
EQ_WINDOW_IATTR_PLANES_DEPTH    { return EQTOKEN_WINDOW_IATTR_PLANES_DEPTH; }
EQ_WINDOW_IATTR_PLANES_STENCIL  { return EQTOKEN_WINDOW_IATTR_PLANES_STENCIL; }
EQ_CHANNEL_IATTR_HINT_STATISTICS { return EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS; }
server                          { return EQTOKEN_SERVER; }
config                          { return EQTOKEN_CONFIG; }
appNode                         { return EQTOKEN_APPNODE; }
node                            { return EQTOKEN_NODE; }
pipe                            { return EQTOKEN_PIPE; }
window                          { return EQTOKEN_WINDOW; }
attributes                      { return EQTOKEN_ATTRIBUTES; }
hint_stereo                     { return EQTOKEN_HINT_STEREO; }
hint_doublebuffer               { return EQTOKEN_HINT_DOUBLEBUFFER; }
hint_fullscreen                 { return EQTOKEN_HINT_FULLSCREEN; }
hint_decoration                 { return EQTOKEN_HINT_DECORATION; }
hint_statistics                 { return EQTOKEN_HINT_STATISTICS; }
planes_color                    { return EQTOKEN_PLANES_COLOR; }
planes_alpha                    { return EQTOKEN_PLANES_ALPHA; }
planes_depth                    { return EQTOKEN_PLANES_DEPTH; }
planes_stencil                  { return EQTOKEN_PLANES_STENCIL; }
on                              { return EQTOKEN_ON; }
off                             { return EQTOKEN_OFF; }
auto                            { return EQTOKEN_AUTO; }
fastest                         { return EQTOKEN_FASTEST; }
nicest                          { return EQTOKEN_NICEST; }
channel                         { return EQTOKEN_CHANNEL; }
compound                        { return EQTOKEN_COMPOUND; }
connection                      { return EQTOKEN_CONNECTION; }
name                            { return EQTOKEN_NAME; }
type                            { return EQTOKEN_TYPE; }
TCPIP|tcpip                     { return EQTOKEN_TCPIP; }
hostname                        { return EQTOKEN_HOSTNAME; }
command                         { return EQTOKEN_COMMAND; }
timeout                         { return EQTOKEN_TIMEOUT; }
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
display                         { return EQTOKEN_DISPLAY; }
screen                          { return EQTOKEN_SCREEN; }
wall                            { return EQTOKEN_WALL; }
bottom_left                     { return EQTOKEN_BOTTOM_LEFT; }
bottom_right                    { return EQTOKEN_BOTTOM_RIGHT; }
top_left                        { return EQTOKEN_TOP_LEFT; }
latency                         { return EQTOKEN_LATENCY; }
swapbarrier                     { return EQTOKEN_SWAPBARRIER; }
outputframe                     { return EQTOKEN_OUTPUTFRAME; }
inputframe                      { return EQTOKEN_INPUTFRAME; }
[+-]?[0-9]+[\.][0-9]*           { return EQTOKEN_FLOAT; }
[+-]?[0-9]*[\.][0-9]+           { return EQTOKEN_FLOAT; }
[+]?[0-9]+                      { return EQTOKEN_UNSIGNED; }
[+-]?[0-9]+                     { return EQTOKEN_INTEGER; }
\"[\-_\.:a-zA-Z0-9]*\"          { return EQTOKEN_STRING; }

[{}\[\]]                        { return *yytext; }
[\n]                            { ++yylineno; }
[ \t]                           { ; }
[/][/]|[#]                      { while( yyinput() != '\n' ); ++yylineno; }

%%
