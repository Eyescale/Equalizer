
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

%%

global                           { return EQTOKEN_GLOBAL; }
EQ_CONNECTION_SATTR_HOSTNAME     { return EQTOKEN_CONNECTION_SATTR_HOSTNAME; }
EQ_CONNECTION_SATTR_LAUNCH_COMMAND { return EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND; }
EQ_CONNECTION_IATTR_TYPE         { return EQTOKEN_CONNECTION_IATTR_TYPE; }
EQ_CONNECTION_IATTR_TCPIP_PORT   { return EQTOKEN_CONNECTION_IATTR_TCPIP_PORT; }
EQ_CONNECTION_IATTR_LAUNCH_TIMEOUT { return EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT; }
EQ_CONFIG_FATTR_EYE_BASE           { return EQTOKEN_CONFIG_FATTR_EYE_BASE; }
EQ_WINDOW_IATTR_HINTS_STEREO       { return EQTOKEN_WINDOW_IATTR_HINTS_STEREO; }
EQ_WINDOW_IATTR_HINTS_DOUBLEBUFFER { return EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER; }
EQ_WINDOW_IATTR_HINTS_FULLSCREEN   { return EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN; }
EQ_WINDOW_IATTR_HINTS_DECORATION   { return EQTOKEN_WINDOW_IATTR_HINTS_DECORATION; }
EQ_WINDOW_IATTR_PLANES_COLOR    { return EQTOKEN_WINDOW_IATTR_PLANES_COLOR; }
EQ_WINDOW_IATTR_PLANES_ALPHA    { return EQTOKEN_WINDOW_IATTR_PLANES_ALPHA; }
EQ_WINDOW_IATTR_PLANES_DEPTH    { return EQTOKEN_WINDOW_IATTR_PLANES_DEPTH; }
EQ_WINDOW_IATTR_PLANES_STENCIL  { return EQTOKEN_WINDOW_IATTR_PLANES_STENCIL; }
server                          { return EQTOKEN_SERVER; }
config                          { return EQTOKEN_CONFIG; }
appNode                         { return EQTOKEN_APPNODE; }
node                            { return EQTOKEN_NODE; }
pipe                            { return EQTOKEN_PIPE; }
window                          { return EQTOKEN_WINDOW; }
attributes                      { return EQTOKEN_ATTRIBUTES; }
hints                           { return EQTOKEN_HINTS; }
stereo                          { return EQTOKEN_STEREO; }
doublebuffer                    { return EQTOKEN_DOUBLEBUFFER; }
fullscreen                      { return EQTOKEN_FULLSCREEN; }
decoration                      { return EQTOKEN_DECORATION; }
planes                          { return EQTOKEN_PLANES; }
color                           { return EQTOKEN_COLOR; }
alpha                           { return EQTOKEN_ALPHA; }
depth                           { return EQTOKEN_DEPTH; }
stencil                         { return EQTOKEN_STENCIL; }
on                              { return EQTOKEN_ON; }
off                             { return EQTOKEN_OFF; }
auto                            { return EQTOKEN_AUTO; }
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
["]                             { 
                                    int i = -1;
                                    while( (yytext[++i] = yyinput()) != '"' )
                                        if( yytext[i] == '\n' )
                                            ++yylineno;
                                    yytext[i] = '\0';
                                    return EQTOKEN_STRING; 
                                }
[+-]?[0-9]+[\.][0-9]*           { return EQTOKEN_FLOAT; }
[+-]?[0-9]*[\.][0-9]+           { return EQTOKEN_FLOAT; }
[+]?[0-9]+                      { return EQTOKEN_UNSIGNED; }
[+-]?[0-9]+                     { return EQTOKEN_INTEGER; }

[{}"\[\]]                       { return *yytext; }
[\n]                            { ++yylineno; }
[ \t]                           { ; }
[/][/]|[#]                      { while( yyinput() != '\n' ); ++yylineno; }

%%

int yywrap(void) {
    return 1;
}
