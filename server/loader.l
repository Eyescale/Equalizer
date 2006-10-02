
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

global                          { return EQTOKEN_GLOBAL; }
EQ_CONNECTION_TYPE              { return EQTOKEN_CONNECTION_TYPE; }
EQ_CONNECTION_HOSTNAME          { return EQTOKEN_CONNECTION_HOSTNAME; }
EQ_CONNECTION_TCPIP_PORT        { return EQTOKEN_CONNECTION_TCPIP_PORT; }
EQ_CONNECTION_LAUNCH_TIMEOUT    { return EQTOKEN_CONNECTION_LAUNCH_TIMEOUT; }
EQ_CONNECTION_LAUNCH_COMMAND    { return EQTOKEN_CONNECTION_LAUNCH_COMMAND; }
EQ_WINDOW_IATTR_HINTS_STEREO    { return EQTOKEN_WINDOW_IATTR_HINTS_STEREO; }
server                          { return EQTOKEN_SERVER; }
config                          { return EQTOKEN_CONFIG; }
appNode                         { return EQTOKEN_APPNODE; }
node                            { return EQTOKEN_NODE; }
pipe                            { return EQTOKEN_PIPE; }
window                          { return EQTOKEN_WINDOW; }
attributes                      { return EQTOKEN_ATTRIBUTES; }
hints                           { return EQTOKEN_HINTS; }
stereo                          { return EQTOKEN_STEREO; }
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
format                          { return EQTOKEN_FORMAT; }
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
