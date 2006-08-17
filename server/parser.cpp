/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse eqLoader_parse
#define yylex   eqLoader_lex
#define yyerror eqLoader_error
#define yylval  eqLoader_lval
#define yychar  eqLoader_char
#define yydebug eqLoader_debug
#define yynerrs eqLoader_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     EQTOKEN_GLOBAL = 258,
     EQTOKEN_CONNECTION_TYPE = 259,
     EQTOKEN_CONNECTION_HOSTNAME = 260,
     EQTOKEN_CONNECTION_TCPIP_PORT = 261,
     EQTOKEN_CONNECTION_LAUNCH_TIMEOUT = 262,
     EQTOKEN_CONNECTION_LAUNCH_COMMAND = 263,
     EQTOKEN_SERVER = 264,
     EQTOKEN_CONFIG = 265,
     EQTOKEN_APPNODE = 266,
     EQTOKEN_NODE = 267,
     EQTOKEN_PIPE = 268,
     EQTOKEN_WINDOW = 269,
     EQTOKEN_CHANNEL = 270,
     EQTOKEN_COMPOUND = 271,
     EQTOKEN_CONNECTION = 272,
     EQTOKEN_NAME = 273,
     EQTOKEN_TYPE = 274,
     EQTOKEN_TCPIP = 275,
     EQTOKEN_HOSTNAME = 276,
     EQTOKEN_COMMAND = 277,
     EQTOKEN_TIMEOUT = 278,
     EQTOKEN_TASK = 279,
     EQTOKEN_CLEAR = 280,
     EQTOKEN_DRAW = 281,
     EQTOKEN_VIEWPORT = 282,
     EQTOKEN_RANGE = 283,
     EQTOKEN_DISPLAY = 284,
     EQTOKEN_WALL = 285,
     EQTOKEN_BOTTOM_LEFT = 286,
     EQTOKEN_BOTTOM_RIGHT = 287,
     EQTOKEN_TOP_LEFT = 288,
     EQTOKEN_SYNC = 289,
     EQTOKEN_LATENCY = 290,
     EQTOKEN_SWAPBARRIER = 291,
     EQTOKEN_OUTPUTFRAME = 292,
     EQTOKEN_INPUTFRAME = 293,
     EQTOKEN_STRING = 294,
     EQTOKEN_FLOAT = 295,
     EQTOKEN_INTEGER = 296,
     EQTOKEN_UNSIGNED = 297
   };
#endif
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_TYPE 259
#define EQTOKEN_CONNECTION_HOSTNAME 260
#define EQTOKEN_CONNECTION_TCPIP_PORT 261
#define EQTOKEN_CONNECTION_LAUNCH_TIMEOUT 262
#define EQTOKEN_CONNECTION_LAUNCH_COMMAND 263
#define EQTOKEN_SERVER 264
#define EQTOKEN_CONFIG 265
#define EQTOKEN_APPNODE 266
#define EQTOKEN_NODE 267
#define EQTOKEN_PIPE 268
#define EQTOKEN_WINDOW 269
#define EQTOKEN_CHANNEL 270
#define EQTOKEN_COMPOUND 271
#define EQTOKEN_CONNECTION 272
#define EQTOKEN_NAME 273
#define EQTOKEN_TYPE 274
#define EQTOKEN_TCPIP 275
#define EQTOKEN_HOSTNAME 276
#define EQTOKEN_COMMAND 277
#define EQTOKEN_TIMEOUT 278
#define EQTOKEN_TASK 279
#define EQTOKEN_CLEAR 280
#define EQTOKEN_DRAW 281
#define EQTOKEN_VIEWPORT 282
#define EQTOKEN_RANGE 283
#define EQTOKEN_DISPLAY 284
#define EQTOKEN_WALL 285
#define EQTOKEN_BOTTOM_LEFT 286
#define EQTOKEN_BOTTOM_RIGHT 287
#define EQTOKEN_TOP_LEFT 288
#define EQTOKEN_SYNC 289
#define EQTOKEN_LATENCY 290
#define EQTOKEN_SWAPBARRIER 291
#define EQTOKEN_OUTPUTFRAME 292
#define EQTOKEN_INPUTFRAME 293
#define EQTOKEN_STRING 294
#define EQTOKEN_FLOAT 295
#define EQTOKEN_INTEGER 296
#define EQTOKEN_UNSIGNED 297




/* Copy the first part of user declarations.  */


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
        static eqs::Loader* loader = NULL;
        static std::string  stringBuf;
        
        static eqs::Server*      server;
        static eqs::Config*      config;
        static eqs::Node*        node;
        static eqs::Pipe*        eqPipe; // avoid name clash with pipe()
        static eqs::Window*      window;
        static eqs::Channel*     channel;
        static eqs::Compound*    compound;
        static eqs::SwapBarrier* swapBarrier;
        static eqs::Frame*       frame;
        static eqBase::RefPtr<eqNet::ConnectionDescription> 
            connectionDescription;
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


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
} YYSTYPE;
/* Line 191 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  18
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   199

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  66
/* YYNRULES -- Number of rules. */
#define YYNRULES  120
/* YYNRULES -- Number of states. */
#define YYNSTATES  211

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   297

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    45,     2,    46,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,     2,    44,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    34,    35,    41,    43,    46,    47,    55,
      56,    58,    61,    64,    66,    69,    71,    73,    74,    82,
      83,    91,    92,    94,    97,    98,    99,   101,   104,   105,
     111,   112,   114,   117,   120,   123,   126,   129,   131,   134,
     135,   142,   143,   145,   148,   151,   154,   156,   159,   160,
     167,   168,   170,   173,   176,   179,   181,   184,   185,   191,
     192,   194,   197,   200,   203,   205,   208,   209,   210,   219,
     220,   222,   223,   225,   228,   231,   234,   235,   241,   244,
     250,   252,   254,   256,   258,   259,   261,   264,   266,   268,
     290,   291,   297,   298,   300,   303,   306,   307,   313,   314,
     320,   321,   323,   326,   329,   336,   338,   340,   342,   344,
     346
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      48,     0,    -1,    49,    52,    -1,     3,    43,    50,    44,
      -1,    -1,    49,    -1,    50,    49,    -1,     4,    51,    -1,
       5,   109,    -1,     6,   112,    -1,     7,   112,    -1,     8,
     109,    -1,    20,    -1,    -1,     9,    43,    53,    54,    44,
      -1,    55,    -1,    54,    55,    -1,    -1,    10,    43,    56,
      57,    59,    87,    44,    -1,    -1,    58,    -1,    57,    58,
      -1,    35,   112,    -1,    60,    -1,    59,    60,    -1,    63,
      -1,    61,    -1,    -1,    12,    43,    62,    67,    65,    72,
      44,    -1,    -1,    11,    43,    64,    67,    65,    72,    44,
      -1,    -1,    66,    -1,    65,    66,    -1,    -1,    -1,    68,
      -1,    67,    68,    -1,    -1,    17,    43,    69,    70,    44,
      -1,    -1,    71,    -1,    70,    71,    -1,    19,    51,    -1,
      21,   109,    -1,    22,   109,    -1,    23,   112,    -1,    73,
      -1,    72,    73,    -1,    -1,    13,    43,    74,    75,    77,
      44,    -1,    -1,    76,    -1,    75,    76,    -1,    29,   112,
      -1,    27,   108,    -1,    78,    -1,    77,    78,    -1,    -1,
      14,    43,    79,    80,    82,    44,    -1,    -1,    81,    -1,
      80,    81,    -1,    18,   109,    -1,    27,   108,    -1,    83,
      -1,    82,    83,    -1,    -1,    15,    43,    84,    85,    44,
      -1,    -1,    86,    -1,    85,    86,    -1,    18,   109,    -1,
      27,   108,    -1,    88,    -1,    87,    88,    -1,    -1,    -1,
      16,    43,    89,    92,    91,    44,    90,    92,    -1,    -1,
      87,    -1,    -1,    93,    -1,    92,    93,    -1,    18,   109,
      -1,    15,   109,    -1,    -1,    24,    45,    94,    95,    46,
      -1,    27,   108,    -1,    28,    45,   110,   110,    46,    -1,
      97,    -1,    98,    -1,   102,    -1,   104,    -1,    -1,    96,
      -1,    95,    96,    -1,    25,    -1,    26,    -1,    30,    43,
      31,    45,   110,   110,   110,    46,    32,    45,   110,   110,
     110,    46,    33,    45,   110,   110,   110,    46,    44,    -1,
      -1,    36,    43,    99,   100,    44,    -1,    -1,   101,    -1,
     100,   101,    -1,    18,   109,    -1,    -1,    37,    43,   103,
     106,    44,    -1,    -1,    38,    43,   105,   106,    44,    -1,
      -1,   107,    -1,   106,   107,    -1,    18,   109,    -1,    45,
     110,   110,   110,   110,    46,    -1,    39,    -1,    40,    -1,
     111,    -1,    41,    -1,   112,    -1,    42,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   110,   110,   112,   113,   116,   116,   119,   124,   129,
     134,   139,   145,   147,   147,   150,   150,   151,   151,   154,
     154,   154,   156,   158,   158,   159,   159,   160,   160,   164,
     164,   168,   168,   168,   169,   172,   176,   176,   178,   177,
     184,   184,   185,   187,   188,   189,   190,   193,   193,   194,
     194,   197,   197,   197,   199,   200,   206,   206,   207,   207,
     210,   210,   210,   212,   213,   222,   222,   223,   223,   226,
     227,   227,   229,   230,   240,   240,   242,   251,   241,   254,
     254,   256,   256,   257,   259,   260,   268,   268,   270,   272,
     274,   275,   276,   277,   279,   279,   279,   281,   282,   284,
     305,   305,   311,   311,   312,   314,   316,   316,   322,   322,
     328,   328,   328,   330,   332,   340,   345,   346,   348,   349,
     350
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQTOKEN_GLOBAL", 
  "EQTOKEN_CONNECTION_TYPE", "EQTOKEN_CONNECTION_HOSTNAME", 
  "EQTOKEN_CONNECTION_TCPIP_PORT", "EQTOKEN_CONNECTION_LAUNCH_TIMEOUT", 
  "EQTOKEN_CONNECTION_LAUNCH_COMMAND", "EQTOKEN_SERVER", "EQTOKEN_CONFIG", 
  "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE", "EQTOKEN_WINDOW", 
  "EQTOKEN_CHANNEL", "EQTOKEN_COMPOUND", "EQTOKEN_CONNECTION", 
  "EQTOKEN_NAME", "EQTOKEN_TYPE", "EQTOKEN_TCPIP", "EQTOKEN_HOSTNAME", 
  "EQTOKEN_COMMAND", "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_CLEAR", 
  "EQTOKEN_DRAW", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE", "EQTOKEN_DISPLAY", 
  "EQTOKEN_WALL", "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT", 
  "EQTOKEN_TOP_LEFT", "EQTOKEN_SYNC", "EQTOKEN_LATENCY", 
  "EQTOKEN_SWAPBARRIER", "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME", 
  "EQTOKEN_STRING", "EQTOKEN_FLOAT", "EQTOKEN_INTEGER", 
  "EQTOKEN_UNSIGNED", "'{'", "'}'", "'['", "']'", "$accept", "file", 
  "global", "globals", "connectionType", "server", "@1", "configs", 
  "config", "@2", "configAttributes", "configAttribute", "nodes", "node", 
  "otherNode", "@3", "appNode", "@4", "nodeAttributes", "nodeAttribute", 
  "connections", "connection", "@5", "connectionAttributes", 
  "connectionAttribute", "pipes", "pipe", "@6", "pipeAttributes", 
  "pipeAttribute", "windows", "window", "@7", "windowAttributes", 
  "windowAttribute", "channels", "channel", "@8", "channelAttributes", 
  "channelAttribute", "compounds", "compound", "@9", "@10", 
  "compoundChildren", "compoundAttributes", "compoundAttribute", "@11", 
  "compoundTasks", "compoundTask", "wall", "swapBarrier", "@12", 
  "swapBarrierAttributes", "swapBarrierAttribute", "outputFrame", "@13", 
  "inputFrame", "@14", "frameAttributes", "frameAttribute", "viewport", 
  "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   123,   125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    47,    48,    49,    49,    50,    50,    49,    49,    49,
      49,    49,    51,    53,    52,    54,    54,    56,    55,    57,
      57,    57,    58,    59,    59,    60,    60,    62,    61,    64,
      63,    65,    65,    65,    66,    67,    67,    67,    69,    68,
      70,    70,    70,    71,    71,    71,    71,    72,    72,    74,
      73,    75,    75,    75,    76,    76,    77,    77,    79,    78,
      80,    80,    80,    81,    81,    82,    82,    84,    83,    85,
      85,    85,    86,    86,    87,    87,    89,    90,    88,    91,
      91,    92,    92,    92,    93,    93,    94,    93,    93,    93,
      93,    93,    93,    93,    95,    95,    95,    96,    96,    97,
      99,    98,   100,   100,   100,   101,   103,   102,   105,   104,
     106,   106,   106,   107,   108,   109,   110,   110,   111,   111,
     112
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     1,     0,     5,     1,     2,     0,     7,     0,
       1,     2,     2,     1,     2,     1,     1,     0,     7,     0,
       7,     0,     1,     2,     0,     0,     1,     2,     0,     5,
       0,     1,     2,     2,     2,     2,     2,     1,     2,     0,
       6,     0,     1,     2,     2,     2,     1,     2,     0,     6,
       0,     1,     2,     2,     2,     1,     2,     0,     5,     0,
       1,     2,     2,     2,     1,     2,     0,     0,     8,     0,
       1,     0,     1,     2,     2,     2,     0,     5,     2,     5,
       1,     1,     1,     1,     0,     1,     2,     1,     1,    21,
       0,     5,     0,     1,     2,     2,     0,     5,     0,     5,
       0,     1,     2,     2,     6,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     4,
      12,     7,   115,     8,   120,     9,    10,    11,     1,     0,
       2,     5,     0,    13,     3,     6,     0,     0,     0,    15,
      17,    14,    16,    19,     0,     0,    20,    22,     0,     0,
      21,     0,    23,    26,    25,    29,    27,     0,    24,     0,
      74,    35,    35,    76,    18,    75,     0,    31,    36,    31,
      81,    38,     0,    32,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    79,    82,    90,    91,    92,
      93,    40,     0,    33,     0,    47,     0,    85,    84,    86,
       0,    88,     0,     0,   100,   106,   108,    80,     0,    83,
       0,     0,     0,     0,     0,    41,    49,    30,    48,    28,
      94,   116,   118,     0,   117,   119,     0,     0,   102,   110,
     110,    77,    43,    44,    45,    46,    39,    42,    51,    97,
      98,     0,    95,     0,     0,     0,     0,     0,   103,     0,
       0,   111,     0,    81,     0,     0,     0,    52,    87,    96,
       0,    89,     0,   105,   101,   104,   113,   107,   112,   109,
      78,    55,    54,     0,    53,     0,    56,     0,     0,    58,
      50,    57,   114,     0,    60,     0,     0,     0,     0,    61,
       0,    63,    64,     0,    62,     0,    65,     0,    67,    59,
      66,     0,    69,     0,     0,     0,     0,    70,     0,    72,
      73,    68,    71,     0,     0,     0,     0,     0,     0,     0,
      99
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     7,     8,    22,    11,    20,    26,    28,    29,    33,
      35,    36,    41,    42,    43,    52,    44,    51,    62,    63,
      57,    58,    81,   104,   105,    84,    85,   128,   146,   147,
     165,   166,   174,   178,   179,   185,   186,   192,   196,   197,
      49,    50,    60,   143,    98,    75,    76,   110,   131,   132,
      77,    78,   118,   137,   138,    79,   119,    80,   120,   140,
     141,    91,    13,   113,   114,   115
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -140
static const short yypact[] =
{
     131,   -21,    16,    -9,    47,    47,    -9,    94,    88,   131,
    -140,  -140,  -140,  -140,  -140,  -140,  -140,  -140,  -140,    65,
    -140,  -140,     4,  -140,  -140,  -140,    92,    66,     5,  -140,
    -140,  -140,  -140,    63,    47,    22,  -140,  -140,    69,    72,
    -140,    95,  -140,  -140,  -140,  -140,  -140,    77,  -140,     7,
    -140,   104,   104,  -140,  -140,  -140,    83,   104,  -140,   104,
      86,  -140,   118,  -140,  -140,   118,    -9,    -9,    87,    96,
     102,    97,   107,   108,   110,    55,  -140,  -140,  -140,  -140,
    -140,   106,   112,  -140,     0,  -140,     6,  -140,  -140,  -140,
     103,  -140,   103,   125,  -140,  -140,  -140,   133,   113,  -140,
      16,    -9,    -9,    47,    46,  -140,  -140,  -140,  -140,  -140,
      -8,  -140,  -140,   103,  -140,  -140,   103,   114,   140,   142,
     142,  -140,  -140,  -140,  -140,  -140,  -140,  -140,    13,  -140,
    -140,    -5,  -140,   103,   115,   103,    -9,     9,  -140,    -9,
      11,  -140,    14,    86,    96,    47,    10,  -140,  -140,  -140,
     103,  -140,   103,  -140,  -140,  -140,  -140,  -140,  -140,  -140,
      86,  -140,  -140,   119,  -140,     2,  -140,   117,   103,  -140,
    -140,  -140,  -140,   120,    59,   132,    -9,    96,    48,  -140,
     122,  -140,  -140,   126,  -140,    -1,  -140,   103,  -140,  -140,
    -140,   103,    60,   103,    -9,    96,     8,  -140,   127,  -140,
    -140,  -140,  -140,   135,   129,   103,   103,   103,   130,   121,
    -140
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -140,  -140,    50,  -140,    71,  -140,  -140,  -140,   144,  -140,
    -140,   143,  -140,   134,  -140,  -140,  -140,  -140,   123,    19,
     128,    17,  -140,  -140,    73,   116,    62,  -140,  -140,    33,
    -140,    18,  -140,  -140,    15,  -140,     1,  -140,  -140,   -12,
     124,   -43,  -140,  -140,  -140,    42,   -72,  -140,  -140,    56,
    -140,  -140,  -140,  -140,    52,  -140,  -140,  -140,  -140,    70,
      12,  -139,    -6,   -88,  -140,    -3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      17,    15,    16,    99,   116,   161,    55,     1,     2,     3,
       4,     5,     6,    82,   183,    27,   163,   129,   130,    82,
     129,   130,     9,    47,   163,   133,   194,   136,   134,   139,
      12,    37,   139,    38,    39,   195,    10,   144,   182,   145,
     144,   148,   145,   189,   107,   150,   170,   152,    24,    31,
     109,    54,   201,   154,    55,   157,   200,    34,   159,    21,
      87,    88,   167,   183,   168,   100,   176,   101,   102,   103,
      66,    47,    25,    67,    64,   177,    64,   176,   194,    68,
     173,    83,    69,    70,    83,    71,   177,   195,    99,    14,
     126,    72,    73,    74,    18,   123,   124,    19,    34,   191,
     125,    66,    27,   193,    67,   198,    38,    39,    23,    30,
      68,    47,    45,    69,    70,    46,    71,   206,   207,   208,
      53,    56,    72,    73,    74,   100,    61,   101,   102,   103,
     153,    82,    89,   156,     1,     2,     3,     4,     5,     6,
      93,    90,   162,   111,   112,    14,   108,    92,   108,    47,
      94,    95,   158,    96,   158,   106,   117,   121,   136,   135,
     139,   151,   169,   172,   180,   210,   175,   187,   204,   188,
     181,   122,    32,   203,   205,    48,   209,   127,    40,   164,
      59,    86,    65,   171,   202,   160,   190,   149,   199,   155,
     142,     0,     0,   184,     0,     0,     0,     0,     0,    97
};

static const short yycheck[] =
{
       6,     4,     5,    75,    92,   144,    49,     3,     4,     5,
       6,     7,     8,    13,    15,    10,    14,    25,    26,    13,
      25,    26,    43,    16,    14,   113,    18,    18,   116,    18,
      39,    34,    18,    11,    12,    27,    20,    27,   177,    29,
      27,    46,    29,    44,    44,   133,    44,   135,    44,    44,
      44,    44,    44,    44,    97,    44,   195,    35,    44,     9,
      66,    67,   150,    15,   152,    19,    18,    21,    22,    23,
      15,    16,    22,    18,    57,    27,    59,    18,    18,    24,
     168,    62,    27,    28,    65,    30,    27,    27,   160,    42,
      44,    36,    37,    38,     0,   101,   102,     9,    35,   187,
     103,    15,    10,   191,    18,   193,    11,    12,    43,    43,
      24,    16,    43,    27,    28,    43,    30,   205,   206,   207,
      43,    17,    36,    37,    38,    19,    43,    21,    22,    23,
     136,    13,    45,   139,     3,     4,     5,     6,     7,     8,
      43,    45,   145,    40,    41,    42,    84,    45,    86,    16,
      43,    43,   140,    43,   142,    43,    31,    44,    18,    45,
      18,    46,    43,    46,    32,    44,    46,    45,    33,    43,
     176,   100,    28,    46,    45,    41,    46,   104,    35,   146,
      52,    65,    59,   165,   196,   143,   185,   131,   194,   137,
     120,    -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,    75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    48,    49,    43,
      20,    51,    39,   109,    42,   112,   112,   109,     0,     9,
      52,    49,    50,    43,    44,    49,    53,    10,    54,    55,
      43,    44,    55,    56,    35,    57,    58,   112,    11,    12,
      58,    59,    60,    61,    63,    43,    43,    16,    60,    87,
      88,    64,    62,    43,    44,    88,    17,    67,    68,    67,
      89,    43,    65,    66,    68,    65,    15,    18,    24,    27,
      28,    30,    36,    37,    38,    92,    93,    97,    98,   102,
     104,    69,    13,    66,    72,    73,    72,   109,   109,    45,
      45,   108,    45,    43,    43,    43,    43,    87,    91,    93,
      19,    21,    22,    23,    70,    71,    43,    44,    73,    44,
      94,    40,    41,   110,   111,   112,   110,    31,    99,   103,
     105,    44,    51,   109,   109,   112,    44,    71,    74,    25,
      26,    95,    96,   110,   110,    45,    18,   100,   101,    18,
     106,   107,   106,    90,    27,    29,    75,    76,    46,    96,
     110,    46,   110,   109,    44,   101,   109,    44,   107,    44,
      92,   108,   112,    14,    76,    77,    78,   110,   110,    43,
      44,    78,    46,   110,    79,    46,    18,    27,    80,    81,
      32,   109,   108,    15,    81,    82,    83,    45,    43,    44,
      83,   110,    84,   110,    18,    27,    85,    86,   110,   109,
     108,    44,    86,    46,    33,    45,   110,   110,   110,    46,
      44
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 7:

    { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, yyvsp[0]._connectionType ); 
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, yyvsp[0]._string );
     ;}
    break;

  case 9:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, yyvsp[0]._unsigned );
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, yyvsp[0]._unsigned );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, yyvsp[0]._string );
     ;}
    break;

  case 12:

    { yyval._connectionType = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 13:

    { server = loader->createServer(); ;}
    break;

  case 17:

    { config = loader->createConfig(); ;}
    break;

  case 18:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 22:

    { config->setLatency( yyvsp[0]._unsigned ); ;}
    break;

  case 27:

    { node = loader->createNode(); ;}
    break;

  case 28:

    { config->addNode( node ); node = NULL; ;}
    break;

  case 29:

    { node = loader->createNode(); ;}
    break;

  case 30:

    { config->addApplicationNode( node ); node = NULL; ;}
    break;

  case 35:

    { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             ;}
    break;

  case 38:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 39:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             ;}
    break;

  case 43:

    { connectionDescription->type = yyvsp[0]._connectionType; ;}
    break;

  case 44:

    { connectionDescription->hostname = yyvsp[0]._string; ;}
    break;

  case 45:

    { connectionDescription->launchCommand = yyvsp[0]._string; ;}
    break;

  case 46:

    { connectionDescription->launchTimeout = yyvsp[0]._unsigned; ;}
    break;

  case 49:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 50:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 54:

    { eqPipe->setDisplay( yyvsp[0]._unsigned ); ;}
    break;

  case 55:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], (int)yyvsp[0]._viewport[1],
                                                      (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
        ;}
    break;

  case 58:

    { window = loader->createWindow(); ;}
    break;

  case 59:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 63:

    { window->setName( yyvsp[0]._string ); ;}
    break;

  case 64:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], 
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                window->setViewport( eq::Viewport(yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); 
        ;}
    break;

  case 67:

    { channel = loader->createChannel(); ;}
    break;

  case 68:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 72:

    { channel->setName( yyvsp[0]._string ); ;}
    break;

  case 73:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0],
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                channel->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3]));
        ;}
    break;

  case 76:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( compound )
                      compound->addChild( child );
                  else
                      config->addCompound( child );
                  compound = child;
              ;}
    break;

  case 77:

    { compound = compound->getParent(); ;}
    break;

  case 84:

    { compound->setName( yyvsp[0]._string ); ;}
    break;

  case 85:

    {
         eqs::Channel* channel = config->findChannel( yyvsp[0]._string );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             compound->setChannel( channel );
    ;}
    break;

  case 86:

    { compound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 88:

    { compound->setViewport( eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3] )); ;}
    break;

  case 89:

    { compound->setRange( eq::Range( yyvsp[-2]._float, yyvsp[-1]._float )); ;}
    break;

  case 97:

    { compound->enableTasks( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 98:

    { compound->enableTasks( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 99:

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
        compound->setWall( wall );
    ;}
    break;

  case 100:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 101:

    { 
            compound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 105:

    { swapBarrier->setName( yyvsp[0]._string ); ;}
    break;

  case 106:

    { frame = new eqs::Frame(); ;}
    break;

  case 107:

    { 
            compound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 108:

    { frame = new eqs::Frame(); ;}
    break;

  case 109:

    { 
            compound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 113:

    { frame->setName( yyvsp[0]._string ); ;}
    break;

  case 114:

    { 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;}
    break;

  case 115:

    {
         stringBuf = yytext;
         yyval._string = stringBuf.c_str(); 
     ;}
    break;

  case 116:

    { yyval._float = atof( yytext ); ;}
    break;

  case 117:

    { yyval._float = yyvsp[0]._int; ;}
    break;

  case 118:

    { yyval._int = atoi( yytext ); ;}
    break;

  case 119:

    { yyval._int = yyvsp[0]._unsigned; ;}
    break;

  case 120:

    { yyval._unsigned = atoi( yytext ); ;}
    break;


    }

/* Line 991 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
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
        return NULL;
    }

    server      = NULL;
    bool retval = true;
    if( eqLoader_parse() )
        retval = false;

    fclose( yyin );
    loader = NULL;
    return server;
}

