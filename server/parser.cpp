
/*  A Bison parser, made from c:\visual studio projects\equalizer\src\server\loader.y with Bison version GNU Bison version 1.24
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
#define	EQTOKEN_COMPOUND_IATTR_STEREO_MODE	273
#define	EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK	274
#define	EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK	275
#define	EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS	276
#define	EQTOKEN_SERVER	277
#define	EQTOKEN_CONFIG	278
#define	EQTOKEN_APPNODE	279
#define	EQTOKEN_NODE	280
#define	EQTOKEN_PIPE	281
#define	EQTOKEN_WINDOW	282
#define	EQTOKEN_ATTRIBUTES	283
#define	EQTOKEN_HINT_STEREO	284
#define	EQTOKEN_HINT_DOUBLEBUFFER	285
#define	EQTOKEN_HINT_FULLSCREEN	286
#define	EQTOKEN_HINT_DECORATION	287
#define	EQTOKEN_HINT_STATISTICS	288
#define	EQTOKEN_PLANES_COLOR	289
#define	EQTOKEN_PLANES_ALPHA	290
#define	EQTOKEN_PLANES_DEPTH	291
#define	EQTOKEN_PLANES_STENCIL	292
#define	EQTOKEN_ON	293
#define	EQTOKEN_OFF	294
#define	EQTOKEN_AUTO	295
#define	EQTOKEN_FASTEST	296
#define	EQTOKEN_NICEST	297
#define	EQTOKEN_QUAD	298
#define	EQTOKEN_ANAGLYPH	299
#define	EQTOKEN_RED	300
#define	EQTOKEN_GREEN	301
#define	EQTOKEN_BLUE	302
#define	EQTOKEN_CHANNEL	303
#define	EQTOKEN_COMPOUND	304
#define	EQTOKEN_CONNECTION	305
#define	EQTOKEN_NAME	306
#define	EQTOKEN_TYPE	307
#define	EQTOKEN_TCPIP	308
#define	EQTOKEN_HOSTNAME	309
#define	EQTOKEN_COMMAND	310
#define	EQTOKEN_TIMEOUT	311
#define	EQTOKEN_TASK	312
#define	EQTOKEN_EYE	313
#define	EQTOKEN_EYE_BASE	314
#define	EQTOKEN_BUFFER	315
#define	EQTOKEN_CLEAR	316
#define	EQTOKEN_DRAW	317
#define	EQTOKEN_ASSEMBLE	318
#define	EQTOKEN_READBACK	319
#define	EQTOKEN_COLOR	320
#define	EQTOKEN_DEPTH	321
#define	EQTOKEN_CYCLOP	322
#define	EQTOKEN_LEFT	323
#define	EQTOKEN_RIGHT	324
#define	EQTOKEN_VIEWPORT	325
#define	EQTOKEN_RANGE	326
#define	EQTOKEN_DISPLAY	327
#define	EQTOKEN_SCREEN	328
#define	EQTOKEN_WALL	329
#define	EQTOKEN_BOTTOM_LEFT	330
#define	EQTOKEN_BOTTOM_RIGHT	331
#define	EQTOKEN_TOP_LEFT	332
#define	EQTOKEN_SYNC	333
#define	EQTOKEN_LATENCY	334
#define	EQTOKEN_SWAPBARRIER	335
#define	EQTOKEN_OUTPUTFRAME	336
#define	EQTOKEN_INPUTFRAME	337
#define	EQTOKEN_STEREO_MODE	338
#define	EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK	339
#define	EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK	340
#define	EQTOKEN_STRING	341
#define	EQTOKEN_FLOAT	342
#define	EQTOKEN_INTEGER	343
#define	EQTOKEN_UNSIGNED	344


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



#define	YYFINAL		328
#define	YYFLAG		-32768
#define	YYNTBASE	94

#define YYTRANSLATE(x) ((unsigned)(x) <= 344 ? yytranslate[x] : 176)

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
    92,     2,    93,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    90,     2,    91,     2,     2,     2,     2,     2,
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
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     8,     9,    11,    14,    17,    20,    23,    26,
    29,    32,    35,    38,    41,    44,    47,    50,    53,    56,
    59,    62,    65,    68,    70,    71,    77,    79,    82,    83,
    89,    90,    92,    95,    97,    99,   102,   107,   108,   110,
   113,   116,   118,   121,   123,   125,   126,   132,   133,   139,
   140,   142,   145,   147,   149,   150,   152,   155,   156,   162,
   163,   165,   168,   171,   174,   177,   180,   182,   185,   186,
   192,   193,   195,   198,   200,   203,   206,   209,   211,   214,
   215,   221,   222,   224,   227,   229,   234,   237,   240,   241,
   243,   246,   249,   252,   255,   258,   261,   264,   267,   270,
   272,   275,   276,   282,   283,   285,   288,   291,   296,   299,
   300,   302,   305,   308,   310,   313,   314,   320,   321,   323,
   326,   328,   331,   334,   335,   341,   342,   348,   349,   355,
   358,   364,   366,   368,   370,   372,   377,   378,   380,   383,
   385,   387,   389,   391,   392,   394,   397,   399,   401,   403,
   404,   406,   409,   411,   413,   435,   436,   442,   443,   445,
   448,   451,   452,   458,   459,   465,   466,   468,   471,   474,
   477,   478,   484,   485,   487,   490,   493,   496,   499,   506,
   510,   511,   513,   516,   518,   520,   522,   524,   526,   528,
   530,   532,   534,   536,   538,   540,   542,   544,   546,   548
};

static const short yyrhs[] = {    95,
    98,     0,     3,    90,    96,    91,     0,     0,    95,     0,
    96,    95,     0,     4,   172,     0,     5,   172,     0,     6,
    97,     0,     7,   175,     0,     8,   175,     0,     9,   173,
     0,    10,   171,     0,    11,   171,     0,    12,   171,     0,
    13,   171,     0,    14,   171,     0,    15,   171,     0,    16,
   171,     0,    17,   171,     0,    21,   171,     0,    18,   171,
     0,    19,   168,     0,    20,   168,     0,    53,     0,     0,
    22,    90,    99,   100,    91,     0,   101,     0,   100,   101,
     0,     0,    23,    90,   102,   103,    91,     0,     0,   104,
     0,   103,   104,     0,   107,     0,   139,     0,    79,   175,
     0,    28,    90,   105,    91,     0,     0,   106,     0,   105,
   106,     0,    59,   173,     0,   108,     0,   107,   108,     0,
   111,     0,   109,     0,     0,    25,    90,   110,   113,    91,
     0,     0,    24,    90,   112,   113,    91,     0,     0,   114,
     0,   113,   114,     0,   115,     0,   120,     0,     0,   116,
     0,   115,   116,     0,     0,    50,    90,   117,   118,    91,
     0,     0,   119,     0,   118,   119,     0,    52,    97,     0,
    54,   172,     0,    55,   172,     0,    56,   175,     0,   121,
     0,   120,   121,     0,     0,    26,    90,   122,   123,    91,
     0,     0,   124,     0,   123,   124,     0,   125,     0,    72,
   175,     0,    73,   175,     0,    70,   167,     0,   126,     0,
   125,   126,     0,     0,    27,    90,   127,   128,    91,     0,
     0,   129,     0,   128,   129,     0,   132,     0,    28,    90,
   130,    91,     0,    51,   172,     0,    70,   167,     0,     0,
   131,     0,   130,   131,     0,    29,   171,     0,    30,   171,
     0,    31,   171,     0,    32,   171,     0,    34,   171,     0,
    35,   171,     0,    36,   171,     0,    37,   171,     0,   133,
     0,   132,   133,     0,     0,    48,    90,   134,   135,    91,
     0,     0,   136,     0,   135,   136,     0,    51,   172,     0,
    28,    90,   137,    91,     0,    70,   167,     0,     0,   138,
     0,   137,   138,     0,    33,   171,     0,   140,     0,   139,
   140,     0,     0,    49,    90,   141,   142,    91,     0,     0,
   143,     0,   142,   143,     0,   140,     0,    51,   172,     0,
    48,   172,     0,     0,    57,    92,   144,   147,    93,     0,
     0,    58,    92,   145,   149,    93,     0,     0,    60,    92,
   146,   151,    93,     0,    70,   167,     0,    71,    92,   173,
   173,    93,     0,   153,     0,   154,     0,   158,     0,   160,
     0,    28,    90,   165,    91,     0,     0,   148,     0,   147,
   148,     0,    61,     0,    62,     0,    63,     0,    64,     0,
     0,   150,     0,   149,   150,     0,    67,     0,    68,     0,
    69,     0,     0,   152,     0,   151,   152,     0,    65,     0,
    66,     0,    74,    90,    75,    92,   173,   173,   173,    93,
    76,    92,   173,   173,   173,    93,    77,    92,   173,   173,
   173,    93,    91,     0,     0,    80,    90,   155,   156,    91,
     0,     0,   157,     0,   156,   157,     0,    51,   172,     0,
     0,    81,    90,   159,   162,    91,     0,     0,    82,    90,
   161,   162,    91,     0,     0,   163,     0,   162,   163,     0,
    51,   172,     0,    70,   167,     0,     0,    60,    92,   164,
   151,    93,     0,     0,   166,     0,   165,   166,     0,    83,
   171,     0,    84,   168,     0,    85,   168,     0,    92,   173,
   173,   173,   173,    93,     0,    92,   169,    93,     0,     0,
   170,     0,   169,   170,     0,    45,     0,    46,     0,    47,
     0,    38,     0,    39,     0,    40,     0,    41,     0,    42,
     0,    43,     0,    44,     0,   174,     0,    86,     0,    87,
     0,   174,     0,    88,     0,   175,     0,    89,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   158,   160,   161,   164,   164,   166,   172,   177,   182,   187,
   192,   197,   202,   207,   212,   217,   222,   227,   232,   237,
   242,   247,   252,   258,   260,   261,   263,   263,   264,   266,
   267,   267,   267,   268,   270,   271,   272,   273,   273,   273,
   274,   278,   278,   279,   279,   280,   282,   290,   292,   293,
   293,   293,   294,   296,   297,   298,   298,   299,   301,   306,
   306,   306,   308,   310,   311,   312,   315,   315,   316,   318,
   319,   319,   319,   320,   322,   323,   324,   330,   330,   331,
   333,   334,   334,   334,   335,   337,   338,   339,   347,   347,
   347,   348,   351,   353,   355,   357,   359,   361,   363,   366,
   366,   367,   369,   370,   371,   371,   372,   374,   376,   384,
   384,   384,   385,   390,   390,   391,   401,   403,   403,   403,
   405,   407,   408,   416,   417,   418,   419,   420,   421,   422,
   424,   426,   427,   428,   429,   430,   432,   432,   432,   433,
   435,   436,   437,   439,   439,   439,   440,   442,   443,   445,
   445,   445,   446,   448,   450,   471,   472,   477,   477,   478,
   479,   482,   483,   488,   489,   494,   494,   494,   495,   497,
   499,   500,   502,   502,   502,   503,   506,   509,   513,   521,
   522,   524,   525,   526,   528,   529,   531,   533,   534,   535,
   536,   537,   538,   539,   541,   548,   549,   551,   552,   553
};

static const char * const yytname[] = {   "$","error","$undefined.","EQTOKEN_GLOBAL",
"EQTOKEN_CONNECTION_SATTR_HOSTNAME","EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND",
"EQTOKEN_CONNECTION_IATTR_TYPE","EQTOKEN_CONNECTION_IATTR_TCPIP_PORT","EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT",
"EQTOKEN_CONFIG_FATTR_EYE_BASE","EQTOKEN_WINDOW_IATTR_HINT_STEREO","EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER",
"EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN","EQTOKEN_WINDOW_IATTR_HINT_DECORATION",
"EQTOKEN_WINDOW_IATTR_PLANES_COLOR","EQTOKEN_WINDOW_IATTR_PLANES_ALPHA","EQTOKEN_WINDOW_IATTR_PLANES_DEPTH",
"EQTOKEN_WINDOW_IATTR_PLANES_STENCIL","EQTOKEN_COMPOUND_IATTR_STEREO_MODE","EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK",
"EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK","EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS",
"EQTOKEN_SERVER","EQTOKEN_CONFIG","EQTOKEN_APPNODE","EQTOKEN_NODE","EQTOKEN_PIPE",
"EQTOKEN_WINDOW","EQTOKEN_ATTRIBUTES","EQTOKEN_HINT_STEREO","EQTOKEN_HINT_DOUBLEBUFFER",
"EQTOKEN_HINT_FULLSCREEN","EQTOKEN_HINT_DECORATION","EQTOKEN_HINT_STATISTICS",
"EQTOKEN_PLANES_COLOR","EQTOKEN_PLANES_ALPHA","EQTOKEN_PLANES_DEPTH","EQTOKEN_PLANES_STENCIL",
"EQTOKEN_ON","EQTOKEN_OFF","EQTOKEN_AUTO","EQTOKEN_FASTEST","EQTOKEN_NICEST",
"EQTOKEN_QUAD","EQTOKEN_ANAGLYPH","EQTOKEN_RED","EQTOKEN_GREEN","EQTOKEN_BLUE",
"EQTOKEN_CHANNEL","EQTOKEN_COMPOUND","EQTOKEN_CONNECTION","EQTOKEN_NAME","EQTOKEN_TYPE",
"EQTOKEN_TCPIP","EQTOKEN_HOSTNAME","EQTOKEN_COMMAND","EQTOKEN_TIMEOUT","EQTOKEN_TASK",
"EQTOKEN_EYE","EQTOKEN_EYE_BASE","EQTOKEN_BUFFER","EQTOKEN_CLEAR","EQTOKEN_DRAW",
"EQTOKEN_ASSEMBLE","EQTOKEN_READBACK","EQTOKEN_COLOR","EQTOKEN_DEPTH","EQTOKEN_CYCLOP",
"EQTOKEN_LEFT","EQTOKEN_RIGHT","EQTOKEN_VIEWPORT","EQTOKEN_RANGE","EQTOKEN_DISPLAY",
"EQTOKEN_SCREEN","EQTOKEN_WALL","EQTOKEN_BOTTOM_LEFT","EQTOKEN_BOTTOM_RIGHT",
"EQTOKEN_TOP_LEFT","EQTOKEN_SYNC","EQTOKEN_LATENCY","EQTOKEN_SWAPBARRIER","EQTOKEN_OUTPUTFRAME",
"EQTOKEN_INPUTFRAME","EQTOKEN_STEREO_MODE","EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK",
"EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK","EQTOKEN_STRING","EQTOKEN_FLOAT","EQTOKEN_INTEGER",
"EQTOKEN_UNSIGNED","'{'","'}'","'['","']'","file","global","globals","connectionType",
"server","@1","configs","config","@2","configFields","configField","configAttributes",
"configAttribute","nodes","node","otherNode","@3","appNode","@4","nodeFields",
"nodeField","connections","connection","@5","connectionFields","connectionField",
"pipes","pipe","@6","pipeFields","pipeField","windows","window","@7","windowFields",
"windowField","windowAttributes","windowAttribute","channels","channel","@8",
"channelFields","channelField","channelAttributes","channelAttribute","compounds",
"compound","@9","compoundFields","compoundField","@10","@11","@12","compoundTasks",
"compoundTask","compoundEyes","compoundEye","buffers","buffer","wall","swapBarrier",
"@13","swapBarrierFields","swapBarrierField","outputFrame","@14","inputFrame",
"@15","frameFields","frameField","@16","compoundAttributes","compoundAttribute",
"viewport","colorMask","colorMaskBits","colorMaskBit","IATTR","STRING","FLOAT",
"INTEGER","UNSIGNED",""
};
#endif

static const short yyr1[] = {     0,
    94,    95,    95,    96,    96,    95,    95,    95,    95,    95,
    95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
    95,    95,    95,    97,    99,    98,   100,   100,   102,   101,
   103,   103,   103,   104,   104,   104,   104,   105,   105,   105,
   106,   107,   107,   108,   108,   110,   109,   112,   111,   113,
   113,   113,   114,   114,   115,   115,   115,   117,   116,   118,
   118,   118,   119,   119,   119,   119,   120,   120,   122,   121,
   123,   123,   123,   124,   124,   124,   124,   125,   125,   127,
   126,   128,   128,   128,   129,   129,   129,   129,   130,   130,
   130,   131,   131,   131,   131,   131,   131,   131,   131,   132,
   132,   134,   133,   135,   135,   135,   136,   136,   136,   137,
   137,   137,   138,   139,   139,   141,   140,   142,   142,   142,
   143,   143,   143,   144,   143,   145,   143,   146,   143,   143,
   143,   143,   143,   143,   143,   143,   147,   147,   147,   148,
   148,   148,   148,   149,   149,   149,   150,   150,   150,   151,
   151,   151,   152,   152,   153,   155,   154,   156,   156,   156,
   157,   159,   158,   161,   160,   162,   162,   162,   163,   163,
   164,   163,   165,   165,   165,   166,   166,   166,   167,   168,
   169,   169,   169,   170,   170,   170,   171,   171,   171,   171,
   171,   171,   171,   171,   172,   173,   173,   174,   174,   175
};

static const short yyr2[] = {     0,
     2,     4,     0,     1,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     1,     0,     5,     1,     2,     0,     5,
     0,     1,     2,     1,     1,     2,     4,     0,     1,     2,
     2,     1,     2,     1,     1,     0,     5,     0,     5,     0,
     1,     2,     1,     1,     0,     1,     2,     0,     5,     0,
     1,     2,     2,     2,     2,     2,     1,     2,     0,     5,
     0,     1,     2,     1,     2,     2,     2,     1,     2,     0,
     5,     0,     1,     2,     1,     4,     2,     2,     0,     1,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
     2,     0,     5,     0,     1,     2,     2,     4,     2,     0,
     1,     2,     2,     1,     2,     0,     5,     0,     1,     2,
     1,     2,     2,     0,     5,     0,     5,     0,     5,     2,
     5,     1,     1,     1,     1,     4,     0,     1,     2,     1,
     1,     1,     1,     0,     1,     2,     1,     1,     1,     0,
     1,     2,     1,     1,    21,     0,     5,     0,     1,     2,
     2,     0,     5,     0,     5,     0,     1,     2,     2,     2,
     0,     5,     0,     1,     2,     2,     2,     2,     6,     3,
     0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     3,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     3,   195,     6,     7,    24,     8,   200,     9,    10,   196,
   198,    11,   197,   199,   187,   188,   189,   190,   191,   192,
   193,    12,   194,    13,    14,    15,    16,    17,    18,    19,
    21,   181,    22,    23,    20,     0,     1,     4,     0,   184,
   185,   186,     0,   182,    25,     2,     5,   180,   183,     0,
     0,     0,    27,    29,    26,    28,    31,     0,     0,     0,
     0,     0,     0,    32,    34,    42,    45,    44,    35,   114,
    48,    46,    38,   116,    36,    30,    33,    43,   115,    50,
    50,     0,     0,    39,   118,     0,     0,     0,    51,    53,
    56,    54,    67,     0,    41,    37,    40,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
     0,   119,   132,   133,   134,   135,    69,    58,    49,    52,
    57,    68,    47,   173,   123,   122,   124,   126,   128,     0,
   130,     0,     0,   156,   162,   164,   117,   120,    71,    60,
     0,     0,     0,     0,   174,   137,   144,   150,     0,     0,
     0,   158,   166,   166,     0,     0,     0,     0,     0,    72,
    74,    78,     0,     0,     0,     0,     0,    61,   176,   177,
   178,   136,   175,   140,   141,   142,   143,     0,   138,   147,
   148,   149,     0,   145,   153,   154,     0,   151,     0,     0,
     0,     0,     0,   159,     0,     0,     0,     0,   167,     0,
    80,    77,    75,    76,    70,    73,    79,    63,    64,    65,
    66,    59,    62,   125,   139,   127,   146,   129,   152,     0,
   131,     0,   161,   157,   160,   169,   171,   170,   163,   168,
   165,    82,     0,     0,   150,     0,     0,     0,     0,     0,
    83,    85,   100,   179,     0,     0,    89,   102,    87,    88,
    81,    84,   101,     0,   172,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    90,   104,     0,    92,    93,    94,
    95,    96,    97,    98,    99,    86,    91,     0,     0,     0,
     0,   105,     0,   110,   107,   109,   103,   106,     0,     0,
     0,   111,     0,   113,   108,   112,     0,     0,     0,     0,
     0,     0,     0,     0,   155,     0,     0,     0
};

static const short yydefgoto[] = {   326,
    20,    59,    26,    57,    70,    72,    73,    77,    83,    84,
   103,   104,    85,    86,    87,   101,    88,   100,   108,   109,
   110,   111,   160,   187,   188,   112,   113,   159,   179,   180,
   181,   182,   252,   260,   261,   284,   285,   262,   263,   286,
   301,   302,   311,   312,    89,    90,   105,   131,   132,   166,
   167,   168,   198,   199,   203,   204,   207,   208,   133,   134,
   172,   213,   214,   135,   173,   136,   174,   218,   219,   255,
   164,   165,   151,    53,    63,    64,    42,    23,    32,    43,
    34
};

static const short yypact[] = {   330,
   -25,    15,    15,    68,    48,    48,    72,    90,    90,    90,
    90,    90,    90,    90,    90,    90,    50,    50,    90,    76,
   330,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   220,-32768,-32768,-32768,    54,-32768,-32768,    16,-32768,
-32768,-32768,    31,-32768,-32768,-32768,-32768,-32768,-32768,   117,
    60,   -11,-32768,-32768,-32768,-32768,    47,    74,    78,    94,
   101,    48,    14,-32768,    59,-32768,-32768,-32768,   131,-32768,
-32768,-32768,   134,-32768,-32768,-32768,-32768,-32768,-32768,    86,
    86,    72,   -12,-32768,   250,   104,   106,   -10,-32768,   147,
-32768,   174,-32768,    -9,-32768,-32768,-32768,   124,    15,    15,
   128,   132,   138,   139,   151,   155,   161,   178,   179,-32768,
   141,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   156,-32768,-32768,-32768,-32768,-32768,    72,
-32768,    72,   207,-32768,-32768,-32768,-32768,-32768,    79,   194,
    90,    50,    50,   143,-32768,   200,   212,   111,    72,    72,
   191,   233,    -3,    -3,   195,   139,    48,    48,    17,-32768,
   259,-32768,    68,    15,    15,    48,   153,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   110,-32768,-32768,
-32768,-32768,   149,-32768,-32768,-32768,    45,-32768,    72,   196,
    72,    15,    13,-32768,    15,   198,   139,    97,-32768,   115,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    72,
-32768,    72,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    71,   199,    72,   111,   197,   201,    15,   139,    22,
-32768,   240,-32768,-32768,   202,    88,   223,-32768,-32768,-32768,
-32768,-32768,-32768,   217,-32768,    90,    90,    90,    90,    90,
    90,    90,    90,    24,-32768,    46,   208,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   204,    15,   139,
    18,-32768,    72,   270,-32768,-32768,-32768,-32768,    72,    90,
    12,-32768,    72,-32768,-32768,-32768,   216,   228,   219,    72,
    72,    72,   224,   221,-32768,   316,   318,-32768
};

static const short yypgoto[] = {-32768,
    41,-32768,   136,-32768,-32768,-32768,   251,-32768,-32768,   239,
-32768,   222,-32768,   241,-32768,-32768,-32768,-32768,   226,     6,
-32768,   218,-32768,-32768,   142,-32768,   242,-32768,-32768,   173,
-32768,   172,-32768,-32768,    95,-32768,    73,-32768,    96,-32768,
-32768,    55,-32768,    49,-32768,    34,-32768,-32768,   230,-32768,
-32768,-32768,-32768,   164,-32768,   160,   109,  -164,-32768,-32768,
-32768,-32768,   146,-32768,-32768,-32768,-32768,   192,  -169,-32768,
-32768,   203,  -165,   -16,-32768,   302,    -6,    -2,   -84,    -7,
     9
};


#define	YYLAST		367


static const short yytable[] = {    33,
    24,    54,    44,    45,    46,    47,    48,    49,    50,    51,
   222,    71,    55,    28,    29,   106,   106,   115,     1,     2,
     3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    78,    79,   107,
   107,    80,   239,   175,   310,   298,   102,   215,   250,   256,
   250,   248,   276,   277,   278,   279,   216,   280,   281,   282,
   283,    58,    81,   212,    21,   169,   217,   170,   299,   257,
    78,    79,   258,   298,    80,    60,    61,    62,   116,    75,
   139,   143,    78,    79,   209,   210,   176,   300,   177,   178,
    95,   259,    82,   270,    33,    81,   299,    56,   256,    67,
    22,   239,   315,   244,    96,   175,    66,   225,   307,   205,
   206,   106,   271,   140,   296,   300,   145,   146,   257,   140,
    25,   258,    99,    68,   240,    82,   242,    35,    36,    37,
    38,    39,    40,    41,   306,   107,    27,   238,   130,    71,
   259,    52,    33,    65,    33,   190,   191,   215,   176,    74,
   177,   178,   205,   206,   189,   253,   216,   254,    30,    31,
    27,    33,    33,    91,   130,   215,   217,    92,   118,   265,
   194,   195,   196,   197,   216,   205,   206,    31,    27,    81,
   275,   229,   230,    93,   217,   223,   224,   249,   119,    81,
    94,   120,   102,   137,   231,   138,   107,   121,   122,   106,
   123,    33,   234,    33,   183,   251,   184,   185,   186,   243,
   124,   125,   246,   144,   126,   200,   201,   202,   309,   147,
   127,   128,   129,   148,   313,   161,   162,   163,   317,   149,
   150,   157,    33,   192,    33,   321,   322,   323,   161,   162,
   163,   236,   152,   232,   153,   183,    33,   184,   185,   186,
   154,   276,   277,   278,   279,   269,   280,   281,   282,   283,
   194,   195,   196,   197,    60,    61,    62,   155,   156,   288,
   289,   290,   291,   292,   293,   294,   295,   118,   200,   201,
   202,   171,   211,   212,   221,   175,   267,   257,   241,   247,
   268,   264,   287,   304,   274,    33,   305,   119,    81,   303,
   120,    33,   310,   314,   319,    33,   121,   122,   318,   123,
   320,   325,    33,    33,    33,   327,   324,   328,   228,   124,
   125,    97,    76,   126,   117,    98,   114,   141,   233,   127,
   128,   129,     1,     2,     3,     4,     5,     6,     7,     8,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,   226,   227,   142,   272,   308,   297,   273,   245,   316,
   158,   235,   237,   266,    69,   220,   193
};

static const short yycheck[] = {     7,
     3,    18,     9,    10,    11,    12,    13,    14,    15,    16,
   176,    23,    19,     5,     6,    26,    26,   102,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,    24,    25,    50,
    50,    28,   207,    27,    33,    28,    59,    51,   218,    28,
   220,   217,    29,    30,    31,    32,    60,    34,    35,    36,
    37,    21,    49,    51,    90,   150,    70,   152,    51,    48,
    24,    25,    51,    28,    28,    45,    46,    47,    91,    91,
    91,    91,    24,    25,   169,   170,    70,    70,    72,    73,
    82,    70,    79,   259,   102,    49,    51,    22,    28,    59,
    86,   266,    91,    91,    91,    27,    91,    91,    91,    65,
    66,    26,    91,   108,    91,    70,   119,   120,    48,   114,
    53,    51,    89,    93,   209,    79,   211,    38,    39,    40,
    41,    42,    43,    44,   300,    50,    89,    93,   105,    23,
    70,    92,   150,    90,   152,   162,   163,    51,    70,    90,
    72,    73,    65,    66,   161,   240,    60,   242,    87,    88,
    89,   169,   170,    90,   131,    51,    70,    90,    28,   254,
    61,    62,    63,    64,    60,    65,    66,    88,    89,    49,
    93,   184,   185,    90,    70,   177,   178,    91,    48,    49,
    90,    51,    59,    90,   186,    90,    50,    57,    58,    26,
    60,   209,    93,   211,    52,    91,    54,    55,    56,   212,
    70,    71,   215,    90,    74,    67,    68,    69,   303,    92,
    80,    81,    82,    92,   309,    83,    84,    85,   313,    92,
    92,    91,   240,    91,   242,   320,   321,   322,    83,    84,
    85,    93,    92,    91,    90,    52,   254,    54,    55,    56,
    90,    29,    30,    31,    32,   258,    34,    35,    36,    37,
    61,    62,    63,    64,    45,    46,    47,    90,    90,   276,
   277,   278,   279,   280,   281,   282,   283,    28,    67,    68,
    69,    75,    92,    51,    90,    27,    90,    48,    93,    92,
    90,    93,    76,    90,    93,   303,   299,    48,    49,    92,
    51,   309,    33,   310,    77,   313,    57,    58,    93,    60,
    92,    91,   320,   321,   322,     0,    93,     0,   183,    70,
    71,    83,    72,    74,   103,    85,   101,   110,   187,    80,
    81,    82,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,   179,   181,   112,   260,   301,   284,   262,   213,   311,
   131,   198,   203,   255,    63,   174,   164
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
{ 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_MODE, yyvsp[0]._int ); 
     ;
    break;}
case 22:
{ 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, yyvsp[0]._unsigned ); 
     ;
    break;}
case 23:
{ 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, yyvsp[0]._unsigned ); 
     ;
    break;}
case 24:
{ yyval._connectionType = eqNet::CONNECTIONTYPE_TCPIP; ;
    break;}
case 25:
{ server = loader->createServer(); ;
    break;}
case 29:
{ config = loader->createConfig(); ;
    break;}
case 30:
{ server->addConfig( config ); config = NULL; ;
    break;}
case 36:
{ config->setLatency( yyvsp[0]._unsigned ); ;
    break;}
case 41:
{ config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, yyvsp[0]._float ); ;
    break;}
case 46:
{ node = loader->createNode(); ;
    break;}
case 47:
{ 
                        if( node->nConnectionDescriptions() == 0 )
                            node->addConnectionDescription(
                                new eqs::ConnectionDescription( ));

                        config->addNode( node );
                        node = 0; 
                   ;
    break;}
case 48:
{ node = loader->createNode(); ;
    break;}
case 49:
{ config->addApplicationNode( node ); node = 0; ;
    break;}
case 58:
{ connectionDescription = new eqs::ConnectionDescription(); ;
    break;}
case 59:
{ 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             ;
    break;}
case 63:
{ connectionDescription->type = yyvsp[0]._connectionType; ;
    break;}
case 64:
{ connectionDescription->hostname = yyvsp[0]._string; ;
    break;}
case 65:
{ connectionDescription->launchCommand = yyvsp[0]._string; ;
    break;}
case 66:
{ connectionDescription->launchTimeout = yyvsp[0]._unsigned; ;
    break;}
case 69:
{ eqPipe = loader->createPipe(); ;
    break;}
case 70:
{ node->addPipe( eqPipe ); eqPipe = 0; ;
    break;}
case 75:
{ eqPipe->setDisplay( yyvsp[0]._unsigned ); ;
    break;}
case 76:
{ eqPipe->setScreen( yyvsp[0]._unsigned ); ;
    break;}
case 77:
{
            eqPipe->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], (int)yyvsp[0]._viewport[1],
                                                      (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
        ;
    break;}
case 80:
{ window = loader->createWindow(); ;
    break;}
case 81:
{ eqPipe->addWindow( window ); window = 0; ;
    break;}
case 87:
{ window->setName( yyvsp[0]._string ); ;
    break;}
case 88:
{
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], 
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                window->setViewport( eq::Viewport(yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); 
        ;
    break;}
case 92:
{ window->setIAttribute( eq::Window::IATTR_HINT_STEREO, yyvsp[0]._int ); ;
    break;}
case 93:
{ window->setIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER, yyvsp[0]._int ); ;
    break;}
case 94:
{ window->setIAttribute( eq::Window::IATTR_HINT_FULLSCREEN, yyvsp[0]._int ); ;
    break;}
case 95:
{ window->setIAttribute( eq::Window::IATTR_HINT_DECORATION, yyvsp[0]._int ); ;
    break;}
case 96:
{ window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, yyvsp[0]._int ); ;
    break;}
case 97:
{ window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, yyvsp[0]._int ); ;
    break;}
case 98:
{ window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, yyvsp[0]._int ); ;
    break;}
case 99:
{ window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, yyvsp[0]._int ); ;
    break;}
case 102:
{ channel = loader->createChannel(); ;
    break;}
case 103:
{ window->addChannel( channel ); channel = 0; ;
    break;}
case 107:
{ channel->setName( yyvsp[0]._string ); ;
    break;}
case 109:
{
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0],
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                channel->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3]));
        ;
    break;}
case 113:
{ channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, yyvsp[0]._int ); ;
    break;}
case 116:
{
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;
    break;}
case 117:
{ eqCompound = eqCompound->getParent(); ;
    break;}
case 122:
{ eqCompound->setName( yyvsp[0]._string ); ;
    break;}
case 123:
{
         eqs::Channel* channel = config->findChannel( yyvsp[0]._string );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;
    break;}
case 124:
{ eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;
    break;}
case 126:
{ eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED );;
    break;}
case 128:
{ flags = eq::Frame::BUFFER_NONE; ;
    break;}
case 129:
{ eqCompound->setBuffers( flags ); flags = 0; ;
    break;}
case 130:
{ eqCompound->setViewport( eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3] ));;
    break;}
case 131:
{ eqCompound->setRange( eq::Range( yyvsp[-2]._float, yyvsp[-1]._float )); ;
    break;}
case 140:
{ eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;
    break;}
case 141:
{ eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;
    break;}
case 142:
{ eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);;
    break;}
case 143:
{ eqCompound->enableTask( eqs::Compound::TASK_READBACK);;
    break;}
case 147:
{ eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;
    break;}
case 148:
{ eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;
    break;}
case 149:
{ eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;
    break;}
case 153:
{ flags |= eq::Frame::BUFFER_COLOR; ;
    break;}
case 154:
{ flags |= eq::Frame::BUFFER_DEPTH; ;
    break;}
case 155:
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
case 156:
{ swapBarrier = new eqs::SwapBarrier(); ;
    break;}
case 157:
{ 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        ;
    break;}
case 161:
{ swapBarrier->setName( yyvsp[0]._string ); ;
    break;}
case 162:
{ frame = new eqs::Frame(); ;
    break;}
case 163:
{ 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        ;
    break;}
case 164:
{ frame = new eqs::Frame(); ;
    break;}
case 165:
{ 
            eqCompound->addInputFrame( frame );
            frame = 0;
        ;
    break;}
case 169:
{ frame->setName( yyvsp[0]._string ); ;
    break;}
case 170:
{ frame->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); ;
    break;}
case 171:
{ flags = eq::Frame::BUFFER_NONE; ;
    break;}
case 172:
{ frame->setBuffers( flags ); flags = 0; ;
    break;}
case 176:
{ eqCompound->setIAttribute( eqs::Compound::IATTR_STEREO_MODE, yyvsp[0]._int ); ;
    break;}
case 177:
{ eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, yyvsp[0]._unsigned ); ;
    break;}
case 178:
{ eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, yyvsp[0]._unsigned ); ;
    break;}
case 179:
{ 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;
    break;}
case 180:
{ yyval._unsigned = yyvsp[-1]._unsigned; ;
    break;}
case 181:
{ yyval._unsigned =eqs::Compound::COLOR_MASK_NONE; ;
    break;}
case 182:
{ yyval._unsigned = yyvsp[0]._unsigned; ;
    break;}
case 183:
{ yyval._unsigned = (yyvsp[-1]._unsigned | yyvsp[0]._unsigned);;
    break;}
case 184:
{ yyval._unsigned = eqs::Compound::COLOR_MASK_RED; ;
    break;}
case 185:
{ yyval._unsigned = eqs::Compound::COLOR_MASK_GREEN; ;
    break;}
case 186:
{ yyval._unsigned = eqs::Compound::COLOR_MASK_BLUE; ;
    break;}
case 187:
{ yyval._int = eq::ON; ;
    break;}
case 188:
{ yyval._int = eq::OFF; ;
    break;}
case 189:
{ yyval._int = eq::AUTO; ;
    break;}
case 190:
{ yyval._int = eq::FASTEST; ;
    break;}
case 191:
{ yyval._int = eq::NICEST; ;
    break;}
case 192:
{ yyval._int = eq::QUAD; ;
    break;}
case 193:
{ yyval._int = eq::ANAGLYPH; ;
    break;}
case 194:
{ yyval._int = yyvsp[0]._int; ;
    break;}
case 195:
{
         stringBuf = yytext;
         stringBuf.erase( 0, 1 );                  // Leading '"'
         stringBuf.erase( stringBuf.size()-1, 1 ); // Trailing '"'
         yyval._string = stringBuf.c_str(); 
     ;
    break;}
case 196:
{ yyval._float = atof( yytext ); ;
    break;}
case 197:
{ yyval._float = yyvsp[0]._int; ;
    break;}
case 198:
{ yyval._int = atoi( yytext ); ;
    break;}
case 199:
{ yyval._int = yyvsp[0]._unsigned; ;
    break;}
case 200:
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
