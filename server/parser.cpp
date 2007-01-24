
/*  A Bison parser, made from c:\documents and settings\eile\my documents\visual studio projects\equalizer\src\server\loader.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse eqLoader_parse
#define yylex eqLoader_lex
#define yyerror eqLoader_error
#define yylval eqLoader_lval
#define yychar eqLoader_char
#define yydebug eqLoader_debug
#define yynerrs eqLoader_nerrs
#define	EQTOKEN_GLOBAL	258
#define	EQTOKEN_CONNECTION_SATTR_HOSTNAME	259
#define	EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND	260
#define	EQTOKEN_CONNECTION_IATTR_TYPE	261
#define	EQTOKEN_CONNECTION_IATTR_TCPIP_PORT	262
#define	EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT	263
#define	EQTOKEN_CONFIG_FATTR_EYE_BASE	264
#define	EQTOKEN_WINDOW_IATTR_HINT_STEREO	265
#define	EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER	266
#define	EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN	267
#define	EQTOKEN_WINDOW_IATTR_HINT_DECORATION	268
#define	EQTOKEN_WINDOW_IATTR_PLANES_COLOR	269
#define	EQTOKEN_WINDOW_IATTR_PLANES_ALPHA	270
#define	EQTOKEN_WINDOW_IATTR_PLANES_DEPTH	271
#define	EQTOKEN_WINDOW_IATTR_PLANES_STENCIL	272
#define	EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS	273
#define	EQTOKEN_SERVER	274
#define	EQTOKEN_CONFIG	275
#define	EQTOKEN_APPNODE	276
#define	EQTOKEN_NODE	277
#define	EQTOKEN_PIPE	278
#define	EQTOKEN_WINDOW	279
#define	EQTOKEN_ATTRIBUTES	280
#define	EQTOKEN_HINT_STEREO	281
#define	EQTOKEN_HINT_DOUBLEBUFFER	282
#define	EQTOKEN_HINT_FULLSCREEN	283
#define	EQTOKEN_HINT_DECORATION	284
#define	EQTOKEN_HINT_STATISTICS	285
#define	EQTOKEN_PLANES_COLOR	286
#define	EQTOKEN_PLANES_ALPHA	287
#define	EQTOKEN_PLANES_DEPTH	288
#define	EQTOKEN_PLANES_STENCIL	289
#define	EQTOKEN_ON	290
#define	EQTOKEN_OFF	291
#define	EQTOKEN_AUTO	292
#define	EQTOKEN_FASTEST	293
#define	EQTOKEN_NICEST	294
#define	EQTOKEN_CHANNEL	295
#define	EQTOKEN_COMPOUND	296
#define	EQTOKEN_CONNECTION	297
#define	EQTOKEN_NAME	298
#define	EQTOKEN_TYPE	299
#define	EQTOKEN_TCPIP	300
#define	EQTOKEN_HOSTNAME	301
#define	EQTOKEN_COMMAND	302
#define	EQTOKEN_TIMEOUT	303
#define	EQTOKEN_TASK	304
#define	EQTOKEN_EYE	305
#define	EQTOKEN_EYE_BASE	306
#define	EQTOKEN_BUFFER	307
#define	EQTOKEN_CLEAR	308
#define	EQTOKEN_DRAW	309
#define	EQTOKEN_ASSEMBLE	310
#define	EQTOKEN_READBACK	311
#define	EQTOKEN_COLOR	312
#define	EQTOKEN_DEPTH	313
#define	EQTOKEN_CYCLOP	314
#define	EQTOKEN_LEFT	315
#define	EQTOKEN_RIGHT	316
#define	EQTOKEN_VIEWPORT	317
#define	EQTOKEN_RANGE	318
#define	EQTOKEN_DISPLAY	319
#define	EQTOKEN_SCREEN	320
#define	EQTOKEN_WALL	321
#define	EQTOKEN_BOTTOM_LEFT	322
#define	EQTOKEN_BOTTOM_RIGHT	323
#define	EQTOKEN_TOP_LEFT	324
#define	EQTOKEN_SYNC	325
#define	EQTOKEN_LATENCY	326
#define	EQTOKEN_SWAPBARRIER	327
#define	EQTOKEN_OUTPUTFRAME	328
#define	EQTOKEN_INPUTFRAME	329
#define	EQTOKEN_STRING	330
#define	EQTOKEN_FLOAT	331
#define	EQTOKEN_INTEGER	332
#define	EQTOKEN_UNSIGNED	333


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
        static uint32_t          flags = 0;
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

typedef union{
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::ConnectionType   _connectionType;
    float                   _viewport[4];
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		300
#define	YYFLAG		-32768
#define	YYNTBASE	83

#define YYTRANSLATE(x) ((unsigned)(x) <= 333 ? yytranslate[x] : 160)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    81,     2,    82,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    79,     2,    80,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     8,     9,    11,    14,    17,    20,    23,    26,
    29,    32,    35,    38,    41,    44,    47,    50,    53,    56,
    59,    61,    62,    68,    70,    73,    74,    80,    81,    83,
    86,    88,    90,    93,    98,    99,   101,   104,   107,   109,
   112,   114,   116,   117,   123,   124,   130,   131,   133,   136,
   138,   140,   141,   143,   146,   147,   153,   154,   156,   159,
   162,   165,   168,   171,   173,   176,   177,   183,   184,   186,
   189,   191,   194,   197,   200,   202,   205,   206,   212,   213,
   215,   218,   220,   225,   228,   231,   232,   234,   237,   240,
   243,   246,   249,   252,   255,   258,   261,   263,   266,   267,
   273,   274,   276,   279,   282,   287,   290,   291,   293,   296,
   299,   301,   304,   305,   311,   312,   314,   317,   319,   322,
   325,   326,   332,   333,   339,   340,   346,   349,   355,   357,
   359,   361,   363,   364,   366,   369,   371,   373,   375,   377,
   378,   380,   383,   385,   387,   389,   390,   392,   395,   397,
   399,   421,   422,   428,   429,   431,   434,   437,   438,   444,
   445,   451,   452,   454,   457,   460,   463,   464,   470,   477,
   479,   481,   483,   485,   487,   489,   491,   493,   495,   497,
   499
};

static const short yyrhs[] = {    84,
    87,     0,     3,    79,    85,    80,     0,     0,    84,     0,
    85,    84,     0,     4,   156,     0,     5,   156,     0,     6,
    86,     0,     7,   159,     0,     8,   159,     0,     9,   157,
     0,    10,   155,     0,    11,   155,     0,    12,   155,     0,
    13,   155,     0,    14,   155,     0,    15,   155,     0,    16,
   155,     0,    17,   155,     0,    18,   155,     0,    45,     0,
     0,    19,    79,    88,    89,    80,     0,    90,     0,    89,
    90,     0,     0,    20,    79,    91,    92,    80,     0,     0,
    93,     0,    92,    93,     0,    96,     0,   128,     0,    71,
   159,     0,    25,    79,    94,    80,     0,     0,    95,     0,
    94,    95,     0,    51,   157,     0,    97,     0,    96,    97,
     0,   100,     0,    98,     0,     0,    22,    79,    99,   102,
    80,     0,     0,    21,    79,   101,   102,    80,     0,     0,
   103,     0,   102,   103,     0,   104,     0,   109,     0,     0,
   105,     0,   104,   105,     0,     0,    42,    79,   106,   107,
    80,     0,     0,   108,     0,   107,   108,     0,    44,    86,
     0,    46,   156,     0,    47,   156,     0,    48,   159,     0,
   110,     0,   109,   110,     0,     0,    23,    79,   111,   112,
    80,     0,     0,   113,     0,   112,   113,     0,   114,     0,
    64,   159,     0,    65,   159,     0,    62,   154,     0,   115,
     0,   114,   115,     0,     0,    24,    79,   116,   117,    80,
     0,     0,   118,     0,   117,   118,     0,   121,     0,    25,
    79,   119,    80,     0,    43,   156,     0,    62,   154,     0,
     0,   120,     0,   119,   120,     0,    26,   155,     0,    27,
   155,     0,    28,   155,     0,    29,   155,     0,    31,   155,
     0,    32,   155,     0,    33,   155,     0,    34,   155,     0,
   122,     0,   121,   122,     0,     0,    40,    79,   123,   124,
    80,     0,     0,   125,     0,   124,   125,     0,    43,   156,
     0,    25,    79,   126,    80,     0,    62,   154,     0,     0,
   127,     0,   126,   127,     0,    30,   155,     0,   129,     0,
   128,   129,     0,     0,    41,    79,   130,   131,    80,     0,
     0,   132,     0,   131,   132,     0,   129,     0,    43,   156,
     0,    40,   156,     0,     0,    49,    81,   133,   136,    82,
     0,     0,    50,    81,   134,   138,    82,     0,     0,    52,
    81,   135,   140,    82,     0,    62,   154,     0,    63,    81,
   157,   157,    82,     0,   142,     0,   143,     0,   147,     0,
   149,     0,     0,   137,     0,   136,   137,     0,    53,     0,
    54,     0,    55,     0,    56,     0,     0,   139,     0,   138,
   139,     0,    59,     0,    60,     0,    61,     0,     0,   141,
     0,   140,   141,     0,    57,     0,    58,     0,    66,    79,
    67,    81,   157,   157,   157,    82,    68,    81,   157,   157,
   157,    82,    69,    81,   157,   157,   157,    82,    80,     0,
     0,    72,    79,   144,   145,    80,     0,     0,   146,     0,
   145,   146,     0,    43,   156,     0,     0,    73,    79,   148,
   151,    80,     0,     0,    74,    79,   150,   151,    80,     0,
     0,   152,     0,   151,   152,     0,    43,   156,     0,    62,
   154,     0,     0,    52,    81,   153,   140,    82,     0,    81,
   157,   157,   157,   157,    82,     0,    35,     0,    36,     0,
    37,     0,    38,     0,    39,     0,   158,     0,    75,     0,
    76,     0,   158,     0,    77,     0,   159,     0,    78,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   147,   149,   150,   153,   153,   155,   161,   166,   171,   176,
   181,   186,   191,   196,   201,   206,   211,   216,   221,   226,
   232,   234,   235,   237,   237,   238,   240,   241,   241,   241,
   242,   244,   245,   246,   247,   247,   247,   248,   252,   252,
   253,   253,   254,   256,   264,   266,   267,   267,   267,   268,
   270,   271,   272,   272,   273,   275,   280,   280,   280,   282,
   284,   285,   286,   289,   289,   290,   292,   293,   293,   293,
   294,   296,   297,   298,   304,   304,   305,   307,   308,   308,
   308,   309,   311,   312,   313,   321,   321,   321,   322,   325,
   327,   329,   331,   333,   335,   337,   340,   340,   341,   343,
   344,   345,   345,   346,   348,   350,   358,   358,   358,   359,
   364,   364,   365,   375,   377,   377,   377,   379,   381,   382,
   390,   391,   392,   393,   394,   395,   396,   398,   400,   401,
   402,   403,   405,   405,   405,   406,   408,   409,   410,   412,
   412,   412,   413,   415,   416,   418,   418,   418,   419,   421,
   423,   444,   445,   450,   450,   451,   452,   455,   456,   461,
   462,   467,   467,   467,   468,   470,   472,   473,   475,   483,
   485,   486,   487,   488,   489,   491,   498,   499,   501,   502,
   503
};

static const char * const yytname[] = {   "$","error","$undefined.","EQTOKEN_GLOBAL",
"EQTOKEN_CONNECTION_SATTR_HOSTNAME","EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND",
"EQTOKEN_CONNECTION_IATTR_TYPE","EQTOKEN_CONNECTION_IATTR_TCPIP_PORT","EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT",
"EQTOKEN_CONFIG_FATTR_EYE_BASE","EQTOKEN_WINDOW_IATTR_HINT_STEREO","EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER",
"EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN","EQTOKEN_WINDOW_IATTR_HINT_DECORATION",
"EQTOKEN_WINDOW_IATTR_PLANES_COLOR","EQTOKEN_WINDOW_IATTR_PLANES_ALPHA","EQTOKEN_WINDOW_IATTR_PLANES_DEPTH",
"EQTOKEN_WINDOW_IATTR_PLANES_STENCIL","EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS",
"EQTOKEN_SERVER","EQTOKEN_CONFIG","EQTOKEN_APPNODE","EQTOKEN_NODE","EQTOKEN_PIPE",
"EQTOKEN_WINDOW","EQTOKEN_ATTRIBUTES","EQTOKEN_HINT_STEREO","EQTOKEN_HINT_DOUBLEBUFFER",
"EQTOKEN_HINT_FULLSCREEN","EQTOKEN_HINT_DECORATION","EQTOKEN_HINT_STATISTICS",
"EQTOKEN_PLANES_COLOR","EQTOKEN_PLANES_ALPHA","EQTOKEN_PLANES_DEPTH","EQTOKEN_PLANES_STENCIL",
"EQTOKEN_ON","EQTOKEN_OFF","EQTOKEN_AUTO","EQTOKEN_FASTEST","EQTOKEN_NICEST",
"EQTOKEN_CHANNEL","EQTOKEN_COMPOUND","EQTOKEN_CONNECTION","EQTOKEN_NAME","EQTOKEN_TYPE",
"EQTOKEN_TCPIP","EQTOKEN_HOSTNAME","EQTOKEN_COMMAND","EQTOKEN_TIMEOUT","EQTOKEN_TASK",
"EQTOKEN_EYE","EQTOKEN_EYE_BASE","EQTOKEN_BUFFER","EQTOKEN_CLEAR","EQTOKEN_DRAW",
"EQTOKEN_ASSEMBLE","EQTOKEN_READBACK","EQTOKEN_COLOR","EQTOKEN_DEPTH","EQTOKEN_CYCLOP",
"EQTOKEN_LEFT","EQTOKEN_RIGHT","EQTOKEN_VIEWPORT","EQTOKEN_RANGE","EQTOKEN_DISPLAY",
"EQTOKEN_SCREEN","EQTOKEN_WALL","EQTOKEN_BOTTOM_LEFT","EQTOKEN_BOTTOM_RIGHT",
"EQTOKEN_TOP_LEFT","EQTOKEN_SYNC","EQTOKEN_LATENCY","EQTOKEN_SWAPBARRIER","EQTOKEN_OUTPUTFRAME",
"EQTOKEN_INPUTFRAME","EQTOKEN_STRING","EQTOKEN_FLOAT","EQTOKEN_INTEGER","EQTOKEN_UNSIGNED",
"'{'","'}'","'['","']'","file","global","globals","connectionType","server",
"@1","configs","config","@2","configFields","configField","configAttributes",
"configAttribute","nodes","node","otherNode","@3","appNode","@4","nodeFields",
"nodeField","connections","connection","@5","connectionFields","connectionField",
"pipes","pipe","@6","pipeFields","pipeField","windows","window","@7","windowFields",
"windowField","windowAttributes","windowAttribute","channels","channel","@8",
"channelFields","channelField","channelAttributes","channelAttribute","compounds",
"compound","@9","compoundFields","compoundField","@10","@11","@12","compoundTasks",
"compoundTask","compoundEyes","compoundEye","buffers","buffer","wall","swapBarrier",
"@13","swapBarrierFields","swapBarrierField","outputFrame","@14","inputFrame",
"@15","frameFields","frameField","@16","viewport","IATTR","STRING","FLOAT","INTEGER",
"UNSIGNED",""
};
#endif

static const short yyr1[] = {     0,
    83,    84,    84,    85,    85,    84,    84,    84,    84,    84,
    84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
    86,    88,    87,    89,    89,    91,    90,    92,    92,    92,
    93,    93,    93,    93,    94,    94,    94,    95,    96,    96,
    97,    97,    99,    98,   101,   100,   102,   102,   102,   103,
   103,   104,   104,   104,   106,   105,   107,   107,   107,   108,
   108,   108,   108,   109,   109,   111,   110,   112,   112,   112,
   113,   113,   113,   113,   114,   114,   116,   115,   117,   117,
   117,   118,   118,   118,   118,   119,   119,   119,   120,   120,
   120,   120,   120,   120,   120,   120,   121,   121,   123,   122,
   124,   124,   124,   125,   125,   125,   126,   126,   126,   127,
   128,   128,   130,   129,   131,   131,   131,   132,   132,   132,
   133,   132,   134,   132,   135,   132,   132,   132,   132,   132,
   132,   132,   136,   136,   136,   137,   137,   137,   137,   138,
   138,   138,   139,   139,   139,   140,   140,   140,   141,   141,
   142,   144,   143,   145,   145,   145,   146,   148,   147,   150,
   149,   151,   151,   151,   152,   152,   153,   152,   154,   155,
   155,   155,   155,   155,   155,   156,   157,   157,   158,   158,
   159
};

static const short yyr2[] = {     0,
     2,     4,     0,     1,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     1,     0,     5,     1,     2,     0,     5,     0,     1,     2,
     1,     1,     2,     4,     0,     1,     2,     2,     1,     2,
     1,     1,     0,     5,     0,     5,     0,     1,     2,     1,
     1,     0,     1,     2,     0,     5,     0,     1,     2,     2,
     2,     2,     2,     1,     2,     0,     5,     0,     1,     2,
     1,     2,     2,     2,     1,     2,     0,     5,     0,     1,
     2,     1,     4,     2,     2,     0,     1,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     1,     2,     0,     5,
     0,     1,     2,     2,     4,     2,     0,     1,     2,     2,
     1,     2,     0,     5,     0,     1,     2,     1,     2,     2,
     0,     5,     0,     5,     0,     5,     2,     5,     1,     1,
     1,     1,     0,     1,     2,     1,     1,     1,     1,     0,
     1,     2,     1,     1,     1,     0,     1,     2,     1,     1,
    21,     0,     5,     0,     1,     2,     2,     0,     5,     0,
     5,     0,     1,     2,     2,     2,     0,     5,     6,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1
};

static const short yydefact[] = {     3,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     3,   176,     6,
     7,    21,     8,   181,     9,    10,   177,   179,    11,   178,
   180,   170,   171,   172,   173,   174,    12,   175,    13,    14,
    15,    16,    17,    18,    19,    20,     0,     1,     4,     0,
    22,     2,     5,     0,     0,     0,    24,    26,    23,    25,
    28,     0,     0,     0,     0,     0,     0,    29,    31,    39,
    42,    41,    32,   111,    45,    43,    35,   113,    33,    27,
    30,    40,   112,    47,    47,     0,     0,    36,   115,     0,
     0,     0,    48,    50,    53,    51,    64,     0,    38,    34,
    37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   118,     0,   116,   129,   130,   131,   132,    66,
    55,    46,    49,    54,    65,    44,   120,   119,   121,   123,
   125,     0,   127,     0,     0,   152,   158,   160,   114,   117,
    68,    57,   133,   140,   146,     0,     0,     0,   154,   162,
   162,     0,     0,     0,     0,     0,    69,    71,    75,     0,
     0,     0,     0,     0,    58,   136,   137,   138,   139,     0,
   134,   143,   144,   145,     0,   141,   149,   150,     0,   147,
     0,     0,     0,     0,     0,   155,     0,     0,     0,     0,
   163,     0,    77,    74,    72,    73,    67,    70,    76,    60,
    61,    62,    63,    56,    59,   122,   135,   124,   142,   126,
   148,     0,   128,     0,   157,   153,   156,   165,   167,   166,
   159,   164,   161,    79,     0,     0,   146,     0,     0,     0,
     0,     0,    80,    82,    97,   169,     0,     0,    86,    99,
    84,    85,    78,    81,    98,     0,   168,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    87,   101,     0,    89,
    90,    91,    92,    93,    94,    95,    96,    83,    88,     0,
     0,     0,     0,   102,     0,   107,   104,   106,   100,   103,
     0,     0,     0,   108,     0,   110,   105,   109,     0,     0,
     0,     0,     0,     0,     0,     0,   151,     0,     0,     0
};

static const short yydefgoto[] = {   298,
    17,    50,    23,    48,    54,    56,    57,    61,    67,    68,
    87,    88,    69,    70,    71,    85,    72,    84,    92,    93,
    94,    95,   142,   164,   165,    96,    97,   141,   156,   157,
   158,   159,   224,   232,   233,   256,   257,   234,   235,   258,
   273,   274,   283,   284,    73,    74,    89,   114,   115,   143,
   144,   145,   170,   171,   175,   176,   179,   180,   116,   117,
   149,   185,   186,   118,   150,   119,   151,   190,   191,   227,
   133,    37,    20,    29,    38,    31
};

static const short yypact[] = {   285,
   -20,     6,     6,    46,    30,    30,   157,   109,   109,   109,
   109,   109,   109,   109,   109,   109,    50,   285,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    42,-32768,-32768,    14,
-32768,-32768,-32768,   115,    58,    -9,-32768,-32768,-32768,-32768,
    19,    75,    94,    99,   105,    30,    12,-32768,   128,-32768,
-32768,-32768,    88,-32768,-32768,-32768,    92,-32768,-32768,-32768,
-32768,-32768,-32768,    96,    96,   157,    51,-32768,   189,   116,
   125,    15,-32768,   126,-32768,   152,-32768,    16,-32768,-32768,
-32768,     6,     6,    98,   107,   113,   130,   142,   148,   158,
   161,   171,-32768,   140,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   157,-32768,   157,   164,-32768,-32768,-32768,-32768,-32768,
    41,   178,   162,   197,   100,   157,   157,   155,   210,    25,
    25,   175,   130,    30,    30,   -10,-32768,   235,-32768,    46,
     6,     6,    30,    76,-32768,-32768,-32768,-32768,-32768,   111,
-32768,-32768,-32768,-32768,   139,-32768,-32768,-32768,    95,-32768,
   157,   182,   157,     6,     2,-32768,     6,   179,   130,     4,
-32768,    89,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   157,-32768,   157,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    93,   183,   157,   100,   187,   188,     6,
   130,    24,-32768,   230,-32768,-32768,   190,   114,   278,-32768,
-32768,-32768,-32768,-32768,-32768,   203,-32768,   109,   109,   109,
   109,   109,   109,   109,   109,    83,-32768,    64,   192,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   196,
     6,   130,    18,-32768,   157,   247,-32768,-32768,-32768,-32768,
   157,   109,     5,-32768,   157,-32768,-32768,-32768,   198,   212,
   201,   157,   157,   157,   202,   199,-32768,   283,   308,-32768
};

static const short yypgoto[] = {-32768,
    28,-32768,   153,-32768,-32768,-32768,   258,-32768,-32768,   248,
-32768,   229,-32768,   249,-32768,-32768,-32768,-32768,   232,   -56,
-32768,   225,-32768,-32768,   156,-32768,   226,-32768,-32768,   165,
-32768,   166,-32768,-32768,    91,-32768,    69,-32768,    97,-32768,
-32768,    53,-32768,    44,-32768,    -1,-32768,-32768,   214,-32768,
-32768,-32768,-32768,   159,-32768,   160,   103,  -163,-32768,-32768,
-32768,-32768,   147,-32768,-32768,-32768,-32768,   185,  -116,-32768,
  -138,    -6,    -2,   -84,    -7,     7
};


#define	YYLAST		336


static const short yytable[] = {    30,
    21,    99,    39,    40,    41,    42,    43,    44,    45,    46,
    55,    25,    26,   152,   194,   211,     1,     2,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    62,    63,   282,   123,    64,    90,    90,    62,
    63,   123,   270,    64,   184,    49,   187,   146,   228,   147,
   220,   153,    65,   154,   155,   188,    91,    91,    18,    65,
   271,   181,   182,   229,   152,   189,   230,   187,    47,   197,
    59,    83,    79,   222,   211,   222,   188,    53,    30,   272,
    19,   216,    66,   221,   287,   231,   189,   113,   270,    66,
    22,    80,   242,    52,   122,   126,   212,   279,   214,   127,
   128,    86,   153,   243,   154,   155,   271,    24,   248,   249,
   250,   251,   113,   252,   253,   254,   255,   228,    90,   160,
    51,   161,   162,   163,    30,   272,    30,   225,    65,   226,
   100,   187,   229,   278,    55,   230,    58,    91,    30,    30,
   188,   237,    86,    32,    33,    34,    35,    36,    62,    63,
   189,   177,   178,    75,   231,   204,   177,   178,   201,   202,
   195,   196,   268,   166,   167,   168,   169,    91,   223,   203,
   177,   178,    76,    30,    90,    30,   210,    77,   129,   102,
    65,   215,   103,    78,   218,    28,    24,   130,   104,   105,
   281,   106,   206,   131,   120,   247,   285,   172,   173,   174,
   289,   107,   108,   121,    30,   109,    30,   293,   294,   295,
   132,   110,   111,   112,   166,   167,   168,   169,    30,   139,
   208,   160,   134,   161,   162,   163,   135,   241,   102,    65,
   148,   103,    27,    28,    24,   183,   136,   104,   105,   137,
   106,   260,   261,   262,   263,   264,   265,   266,   267,   138,
   107,   108,   184,   193,   109,   172,   173,   174,   152,   219,
   110,   111,   112,   213,   236,   239,   240,    30,   277,   229,
   259,   246,   275,    30,   276,   286,   282,    30,   297,   290,
   291,   292,   299,   296,    30,    30,    30,     1,     2,     3,
     4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,   248,   249,   250,   251,   300,   252,   253,
   254,   255,   200,    60,    81,   101,    98,    82,   124,   205,
   198,   125,   244,   199,   269,   280,   288,   140,   207,   238,
   245,   217,     0,     0,   209,   192
};

static const short yycheck[] = {     7,
     3,    86,     9,    10,    11,    12,    13,    14,    15,    16,
    20,     5,     6,    24,   153,   179,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    21,    22,    30,    92,    25,    23,    23,    21,
    22,    98,    25,    25,    43,    18,    43,   132,    25,   134,
   189,    62,    41,    64,    65,    52,    42,    42,    79,    41,
    43,   146,   147,    40,    24,    62,    43,    43,    19,    80,
    80,    73,    66,   190,   238,   192,    52,    50,    86,    62,
    75,    80,    71,    80,    80,    62,    62,    89,    25,    71,
    45,    80,   231,    80,    80,    80,   181,    80,   183,   102,
   103,    51,    62,    80,    64,    65,    43,    78,    26,    27,
    28,    29,   114,    31,    32,    33,    34,    25,    23,    44,
    79,    46,    47,    48,   132,    62,   134,   212,    41,   214,
    80,    43,    40,   272,    20,    43,    79,    42,   146,   147,
    52,   226,    51,    35,    36,    37,    38,    39,    21,    22,
    62,    57,    58,    79,    62,    80,    57,    58,   161,   162,
   154,   155,    80,    53,    54,    55,    56,    42,    80,   163,
    57,    58,    79,   181,    23,   183,    82,    79,    81,    40,
    41,   184,    43,    79,   187,    77,    78,    81,    49,    50,
   275,    52,    82,    81,    79,    82,   281,    59,    60,    61,
   285,    62,    63,    79,   212,    66,   214,   292,   293,   294,
    81,    72,    73,    74,    53,    54,    55,    56,   226,    80,
    82,    44,    81,    46,    47,    48,    79,   230,    40,    41,
    67,    43,    76,    77,    78,    81,    79,    49,    50,    79,
    52,   248,   249,   250,   251,   252,   253,   254,   255,    79,
    62,    63,    43,    79,    66,    59,    60,    61,    24,    81,
    72,    73,    74,    82,    82,    79,    79,   275,   271,    40,
    68,    82,    81,   281,    79,   282,    30,   285,    80,    82,
    69,    81,     0,    82,   292,   293,   294,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    26,    27,    28,    29,     0,    31,    32,
    33,    34,   160,    56,    67,    87,    85,    69,    94,   164,
   156,    96,   232,   158,   256,   273,   283,   114,   170,   227,
   234,   185,    -1,    -1,   175,   151
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */


/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 6:
{
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, yyvsp[0]._string );
     ;
    break;}
case 7:
{
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, yyvsp[0]._string );
     ;
    break;}
case 8:
{ 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, yyvsp[0]._connectionType ); 
     ;
    break;}
case 9:
{
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, yyvsp[0]._unsigned );
     ;
    break;}
case 10:
{
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, yyvsp[0]._unsigned );
     ;
    break;}
case 11:
{
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, yyvsp[0]._float );
     ;
    break;}
case 12:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STEREO, yyvsp[0]._int );
     ;
    break;}
case 13:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DOUBLEBUFFER, yyvsp[0]._int );
     ;
    break;}
case 14:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_FULLSCREEN, yyvsp[0]._int );
     ;
    break;}
case 15:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DECORATION, yyvsp[0]._int );
     ;
    break;}
case 16:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, yyvsp[0]._int );
     ;
    break;}
case 17:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, yyvsp[0]._int );
     ;
    break;}
case 18:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, yyvsp[0]._int );
     ;
    break;}
case 19:
{
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, yyvsp[0]._int );
     ;
    break;}
case 20:
{
         eqs::Global::instance()->setChannelIAttribute(
             eq::Channel::IATTR_HINT_STATISTICS, yyvsp[0]._int );
     ;
    break;}
case 21:
{ yyval._connectionType = eqNet::CONNECTIONTYPE_TCPIP; ;
    break;}
case 22:
{ server = loader->createServer(); ;
    break;}
case 26:
{ config = loader->createConfig(); ;
    break;}
case 27:
{ server->addConfig( config ); config = NULL; ;
    break;}
case 33:
{ config->setLatency( yyvsp[0]._unsigned ); ;
    break;}
case 38:
{ config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, yyvsp[0]._float ); ;
    break;}
case 43:
{ node = loader->createNode(); ;
    break;}
case 44:
{ 
                        if( node->nConnectionDescriptions() == 0 )
                            node->addConnectionDescription(
                                new eqs::ConnectionDescription( ));

                        config->addNode( node );
                        node = 0; 
                   ;
    break;}
case 45:
{ node = loader->createNode(); ;
    break;}
case 46:
{ config->addApplicationNode( node ); node = 0; ;
    break;}
case 55:
{ connectionDescription = new eqs::ConnectionDescription(); ;
    break;}
case 56:
{ 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             ;
    break;}
case 60:
{ connectionDescription->type = yyvsp[0]._connectionType; ;
    break;}
case 61:
{ connectionDescription->hostname = yyvsp[0]._string; ;
    break;}
case 62:
{ connectionDescription->launchCommand = yyvsp[0]._string; ;
    break;}
case 63:
{ connectionDescription->launchTimeout = yyvsp[0]._unsigned; ;
    break;}
case 66:
{ eqPipe = loader->createPipe(); ;
    break;}
case 67:
{ node->addPipe( eqPipe ); eqPipe = 0; ;
    break;}
case 72:
{ eqPipe->setDisplay( yyvsp[0]._unsigned ); ;
    break;}
case 73:
{ eqPipe->setScreen( yyvsp[0]._unsigned ); ;
    break;}
case 74:
{
            eqPipe->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], (int)yyvsp[0]._viewport[1],
                                                      (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
        ;
    break;}
case 77:
{ window = loader->createWindow(); ;
    break;}
case 78:
{ eqPipe->addWindow( window ); window = 0; ;
    break;}
case 84:
{ window->setName( yyvsp[0]._string ); ;
    break;}
case 85:
{
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], 
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                window->setViewport( eq::Viewport(yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); 
        ;
    break;}
case 89:
{ window->setIAttribute( eq::Window::IATTR_HINT_STEREO, yyvsp[0]._int ); ;
    break;}
case 90:
{ window->setIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER, yyvsp[0]._int ); ;
    break;}
case 91:
{ window->setIAttribute( eq::Window::IATTR_HINT_FULLSCREEN, yyvsp[0]._int ); ;
    break;}
case 92:
{ window->setIAttribute( eq::Window::IATTR_HINT_DECORATION, yyvsp[0]._int ); ;
    break;}
case 93:
{ window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, yyvsp[0]._int ); ;
    break;}
case 94:
{ window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, yyvsp[0]._int ); ;
    break;}
case 95:
{ window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, yyvsp[0]._int ); ;
    break;}
case 96:
{ window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, yyvsp[0]._int ); ;
    break;}
case 99:
{ channel = loader->createChannel(); ;
    break;}
case 100:
{ window->addChannel( channel ); channel = 0; ;
    break;}
case 104:
{ channel->setName( yyvsp[0]._string ); ;
    break;}
case 106:
{
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0],
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                channel->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3]));
        ;
    break;}
case 110:
{ channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, yyvsp[0]._int ); ;
    break;}
case 113:
{
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;
    break;}
case 114:
{ eqCompound = eqCompound->getParent(); ;
    break;}
case 119:
{ eqCompound->setName( yyvsp[0]._string ); ;
    break;}
case 120:
{
         eqs::Channel* channel = config->findChannel( yyvsp[0]._string );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;
    break;}
case 121:
{ eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;
    break;}
case 123:
{ eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED );;
    break;}
case 125:
{ flags = eq::Frame::BUFFER_NONE; ;
    break;}
case 126:
{ eqCompound->setBuffers( flags ); flags = 0; ;
    break;}
case 127:
{ eqCompound->setViewport( eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3] ));;
    break;}
case 128:
{ eqCompound->setRange( eq::Range( yyvsp[-2]._float, yyvsp[-1]._float )); ;
    break;}
case 136:
{ eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;
    break;}
case 137:
{ eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;
    break;}
case 138:
{ eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);;
    break;}
case 139:
{ eqCompound->enableTask( eqs::Compound::TASK_READBACK);;
    break;}
case 143:
{ eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;
    break;}
case 144:
{ eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;
    break;}
case 145:
{ eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;
    break;}
case 149:
{ flags |= eq::Frame::BUFFER_COLOR; ;
    break;}
case 150:
{ flags |= eq::Frame::BUFFER_DEPTH; ;
    break;}
case 151:
{ 
        eq::Wall wall;
        wall.bottomLeft[0] = yyvsp[-16]._float;
        wall.bottomLeft[1] = yyvsp[-15]._float;
        wall.bottomLeft[2] = yyvsp[-14]._float;

        wall.bottomRight[0] = yyvsp[-10]._float;
        wall.bottomRight[1] = yyvsp[-9]._float;
        wall.bottomRight[2] = yyvsp[-8]._float;

        wall.topLeft[0] = yyvsp[-4]._float;
        wall.topLeft[1] = yyvsp[-3]._float;
        wall.topLeft[2] = yyvsp[-2]._float;
        eqCompound->setWall( wall );
    ;
    break;}
case 152:
{ swapBarrier = new eqs::SwapBarrier(); ;
    break;}
case 153:
{ 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        ;
    break;}
case 157:
{ swapBarrier->setName( yyvsp[0]._string ); ;
    break;}
case 158:
{ frame = new eqs::Frame(); ;
    break;}
case 159:
{ 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        ;
    break;}
case 160:
{ frame = new eqs::Frame(); ;
    break;}
case 161:
{ 
            eqCompound->addInputFrame( frame );
            frame = 0;
        ;
    break;}
case 165:
{ frame->setName( yyvsp[0]._string ); ;
    break;}
case 166:
{ frame->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); ;
    break;}
case 167:
{ flags = eq::Frame::BUFFER_NONE; ;
    break;}
case 168:
{ frame->setBuffers( flags ); flags = 0; ;
    break;}
case 169:
{ 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;
    break;}
case 170:
{ yyval._int = eq::ON; ;
    break;}
case 171:
{ yyval._int = eq::OFF; ;
    break;}
case 172:
{ yyval._int = eq::AUTO; ;
    break;}
case 173:
{ yyval._int = eq::FASTEST; ;
    break;}
case 174:
{ yyval._int = eq::NICEST; ;
    break;}
case 175:
{ yyval._int = yyvsp[0]._int; ;
    break;}
case 176:
{
         stringBuf = yytext;
         stringBuf.erase( 0, 1 );                  // Leading '"'
         stringBuf.erase( stringBuf.size()-1, 1 ); // Trailing '"'
         yyval._string = stringBuf.c_str(); 
     ;
    break;}
case 177:
{ yyval._float = atof( yytext ); ;
    break;}
case 178:
{ yyval._float = yyvsp[0]._int; ;
    break;}
case 179:
{ yyval._int = atoi( yytext ); ;
    break;}
case 180:
{ yyval._int = yyvsp[0]._unsigned; ;
    break;}
case 181:
{ yyval._unsigned = atoi( yytext ); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}


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
        return 0;
    }

    server      = 0;
    bool retval = true;

    if( eqLoader_parse() )
        retval = false;

    fclose( yyin );
    loader = 0;
    return server;
}
