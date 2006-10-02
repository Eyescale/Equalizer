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
     EQTOKEN_WINDOW_IATTR_HINTS_STEREO = 264,
     EQTOKEN_SERVER = 265,
     EQTOKEN_CONFIG = 266,
     EQTOKEN_APPNODE = 267,
     EQTOKEN_NODE = 268,
     EQTOKEN_PIPE = 269,
     EQTOKEN_WINDOW = 270,
     EQTOKEN_ATTRIBUTES = 271,
     EQTOKEN_HINTS = 272,
     EQTOKEN_STEREO = 273,
     EQTOKEN_ON = 274,
     EQTOKEN_OFF = 275,
     EQTOKEN_AUTO = 276,
     EQTOKEN_CHANNEL = 277,
     EQTOKEN_COMPOUND = 278,
     EQTOKEN_CONNECTION = 279,
     EQTOKEN_NAME = 280,
     EQTOKEN_TYPE = 281,
     EQTOKEN_TCPIP = 282,
     EQTOKEN_HOSTNAME = 283,
     EQTOKEN_COMMAND = 284,
     EQTOKEN_TIMEOUT = 285,
     EQTOKEN_TASK = 286,
     EQTOKEN_EYE = 287,
     EQTOKEN_FORMAT = 288,
     EQTOKEN_CLEAR = 289,
     EQTOKEN_DRAW = 290,
     EQTOKEN_READBACK = 291,
     EQTOKEN_CYCLOP = 292,
     EQTOKEN_LEFT = 293,
     EQTOKEN_RIGHT = 294,
     EQTOKEN_COLOR = 295,
     EQTOKEN_DEPTH = 296,
     EQTOKEN_VIEWPORT = 297,
     EQTOKEN_RANGE = 298,
     EQTOKEN_DISPLAY = 299,
     EQTOKEN_WALL = 300,
     EQTOKEN_BOTTOM_LEFT = 301,
     EQTOKEN_BOTTOM_RIGHT = 302,
     EQTOKEN_TOP_LEFT = 303,
     EQTOKEN_SYNC = 304,
     EQTOKEN_LATENCY = 305,
     EQTOKEN_SWAPBARRIER = 306,
     EQTOKEN_OUTPUTFRAME = 307,
     EQTOKEN_INPUTFRAME = 308,
     EQTOKEN_STRING = 309,
     EQTOKEN_FLOAT = 310,
     EQTOKEN_INTEGER = 311,
     EQTOKEN_UNSIGNED = 312
   };
#endif
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_TYPE 259
#define EQTOKEN_CONNECTION_HOSTNAME 260
#define EQTOKEN_CONNECTION_TCPIP_PORT 261
#define EQTOKEN_CONNECTION_LAUNCH_TIMEOUT 262
#define EQTOKEN_CONNECTION_LAUNCH_COMMAND 263
#define EQTOKEN_WINDOW_IATTR_HINTS_STEREO 264
#define EQTOKEN_SERVER 265
#define EQTOKEN_CONFIG 266
#define EQTOKEN_APPNODE 267
#define EQTOKEN_NODE 268
#define EQTOKEN_PIPE 269
#define EQTOKEN_WINDOW 270
#define EQTOKEN_ATTRIBUTES 271
#define EQTOKEN_HINTS 272
#define EQTOKEN_STEREO 273
#define EQTOKEN_ON 274
#define EQTOKEN_OFF 275
#define EQTOKEN_AUTO 276
#define EQTOKEN_CHANNEL 277
#define EQTOKEN_COMPOUND 278
#define EQTOKEN_CONNECTION 279
#define EQTOKEN_NAME 280
#define EQTOKEN_TYPE 281
#define EQTOKEN_TCPIP 282
#define EQTOKEN_HOSTNAME 283
#define EQTOKEN_COMMAND 284
#define EQTOKEN_TIMEOUT 285
#define EQTOKEN_TASK 286
#define EQTOKEN_EYE 287
#define EQTOKEN_FORMAT 288
#define EQTOKEN_CLEAR 289
#define EQTOKEN_DRAW 290
#define EQTOKEN_READBACK 291
#define EQTOKEN_CYCLOP 292
#define EQTOKEN_LEFT 293
#define EQTOKEN_RIGHT 294
#define EQTOKEN_COLOR 295
#define EQTOKEN_DEPTH 296
#define EQTOKEN_VIEWPORT 297
#define EQTOKEN_RANGE 298
#define EQTOKEN_DISPLAY 299
#define EQTOKEN_WALL 300
#define EQTOKEN_BOTTOM_LEFT 301
#define EQTOKEN_BOTTOM_RIGHT 302
#define EQTOKEN_TOP_LEFT 303
#define EQTOKEN_SYNC 304
#define EQTOKEN_LATENCY 305
#define EQTOKEN_SWAPBARRIER 306
#define EQTOKEN_OUTPUTFRAME 307
#define EQTOKEN_INPUTFRAME 308
#define EQTOKEN_STRING 309
#define EQTOKEN_FLOAT 310
#define EQTOKEN_INTEGER 311
#define EQTOKEN_UNSIGNED 312




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
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   248

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  77
/* YYNRULES -- Number of rules. */
#define YYNRULES  151
/* YYNRULES -- Number of states. */
#define YYNSTATES  252

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

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
       2,    60,     2,    61,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,     2,    59,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    37,    39,    41,    43,    44,    50,
      52,    55,    56,    64,    65,    67,    70,    73,    75,    78,
      80,    82,    83,    91,    92,   100,   101,   103,   106,   107,
     108,   110,   113,   114,   120,   121,   123,   126,   129,   132,
     135,   138,   140,   143,   144,   151,   152,   154,   157,   160,
     163,   165,   168,   169,   176,   177,   179,   182,   187,   190,
     193,   194,   196,   199,   204,   205,   207,   210,   213,   215,
     217,   219,   221,   224,   225,   231,   232,   234,   237,   240,
     243,   245,   248,   249,   257,   258,   260,   261,   263,   266,
     269,   272,   273,   279,   280,   286,   287,   293,   296,   302,
     304,   306,   308,   310,   311,   313,   316,   318,   320,   322,
     323,   325,   328,   330,   332,   334,   335,   337,   340,   342,
     344,   366,   367,   373,   374,   376,   379,   382,   383,   389,
     390,   396,   397,   399,   402,   405,   412,   414,   416,   418,
     420,   422
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      63,     0,    -1,    64,    68,    -1,     3,    58,    65,    59,
      -1,    -1,    64,    -1,    65,    64,    -1,     4,    67,    -1,
       5,   135,    -1,     6,   138,    -1,     7,   138,    -1,     8,
     135,    -1,     9,    66,    -1,    19,    -1,    20,    -1,    21,
      -1,    27,    -1,    -1,    10,    58,    69,    70,    59,    -1,
      71,    -1,    70,    71,    -1,    -1,    11,    58,    72,    73,
      75,   108,    59,    -1,    -1,    74,    -1,    73,    74,    -1,
      50,   138,    -1,    76,    -1,    75,    76,    -1,    79,    -1,
      77,    -1,    -1,    13,    58,    78,    83,    81,    88,    59,
      -1,    -1,    12,    58,    80,    83,    81,    88,    59,    -1,
      -1,    82,    -1,    81,    82,    -1,    -1,    -1,    84,    -1,
      83,    84,    -1,    -1,    24,    58,    85,    86,    59,    -1,
      -1,    87,    -1,    86,    87,    -1,    26,    67,    -1,    28,
     135,    -1,    29,   135,    -1,    30,   138,    -1,    89,    -1,
      88,    89,    -1,    -1,    14,    58,    90,    91,    93,    59,
      -1,    -1,    92,    -1,    91,    92,    -1,    44,   138,    -1,
      42,   134,    -1,    94,    -1,    93,    94,    -1,    -1,    15,
      58,    95,    96,   103,    59,    -1,    -1,    97,    -1,    96,
      97,    -1,    16,    58,    98,    59,    -1,    25,   135,    -1,
      42,   134,    -1,    -1,    99,    -1,    98,    99,    -1,    17,
      58,   100,    59,    -1,    -1,   101,    -1,   100,   101,    -1,
      18,   102,    -1,    19,    -1,    20,    -1,    21,    -1,   104,
      -1,   103,   104,    -1,    -1,    22,    58,   105,   106,    59,
      -1,    -1,   107,    -1,   106,   107,    -1,    25,   135,    -1,
      42,   134,    -1,   109,    -1,   108,   109,    -1,    -1,    23,
      58,   110,   112,   111,   112,    59,    -1,    -1,   108,    -1,
      -1,   113,    -1,   112,   113,    -1,    25,   135,    -1,    22,
     135,    -1,    -1,    31,    60,   114,   117,    61,    -1,    -1,
      32,    60,   115,   119,    61,    -1,    -1,    33,    60,   116,
     121,    61,    -1,    42,   134,    -1,    43,    60,   136,   136,
      61,    -1,   123,    -1,   124,    -1,   128,    -1,   130,    -1,
      -1,   118,    -1,   117,   118,    -1,    34,    -1,    35,    -1,
      36,    -1,    -1,   120,    -1,   119,   120,    -1,    37,    -1,
      38,    -1,    39,    -1,    -1,   122,    -1,   121,   122,    -1,
      40,    -1,    41,    -1,    45,    58,    46,    60,   136,   136,
     136,    61,    47,    60,   136,   136,   136,    61,    48,    60,
     136,   136,   136,    61,    59,    -1,    -1,    51,    58,   125,
     126,    59,    -1,    -1,   127,    -1,   126,   127,    -1,    25,
     135,    -1,    -1,    52,    58,   129,   132,    59,    -1,    -1,
      53,    58,   131,   132,    59,    -1,    -1,   133,    -1,   132,
     133,    -1,    25,   135,    -1,    60,   136,   136,   136,   136,
      61,    -1,    54,    -1,    55,    -1,   137,    -1,    56,    -1,
     138,    -1,    57,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   125,   125,   127,   128,   131,   131,   134,   139,   144,
     149,   154,   159,   162,   164,   166,   169,   171,   171,   174,
     174,   175,   175,   178,   178,   178,   180,   182,   182,   183,
     183,   184,   184,   188,   188,   192,   192,   192,   193,   196,
     200,   200,   202,   201,   208,   208,   209,   211,   212,   213,
     214,   217,   217,   218,   218,   221,   221,   221,   223,   224,
     230,   230,   231,   231,   234,   234,   234,   236,   238,   239,
     247,   247,   247,   249,   251,   251,   251,   253,   255,   257,
     259,   262,   262,   263,   263,   266,   267,   267,   269,   270,
     280,   280,   282,   281,   295,   295,   297,   297,   298,   300,
     301,   309,   309,   311,   311,   313,   313,   315,   317,   319,
     320,   321,   322,   324,   324,   324,   326,   327,   328,   330,
     330,   330,   332,   333,   334,   336,   336,   336,   338,   339,
     341,   362,   362,   368,   368,   369,   371,   373,   373,   379,
     379,   385,   385,   385,   387,   389,   397,   402,   403,   405,
     406,   407
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
  "EQTOKEN_CONNECTION_LAUNCH_COMMAND", 
  "EQTOKEN_WINDOW_IATTR_HINTS_STEREO", "EQTOKEN_SERVER", "EQTOKEN_CONFIG", 
  "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE", "EQTOKEN_WINDOW", 
  "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINTS", "EQTOKEN_STEREO", "EQTOKEN_ON", 
  "EQTOKEN_OFF", "EQTOKEN_AUTO", "EQTOKEN_CHANNEL", "EQTOKEN_COMPOUND", 
  "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE", "EQTOKEN_TCPIP", 
  "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND", "EQTOKEN_TIMEOUT", 
  "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_FORMAT", "EQTOKEN_CLEAR", 
  "EQTOKEN_DRAW", "EQTOKEN_READBACK", "EQTOKEN_CYCLOP", "EQTOKEN_LEFT", 
  "EQTOKEN_RIGHT", "EQTOKEN_COLOR", "EQTOKEN_DEPTH", "EQTOKEN_VIEWPORT", 
  "EQTOKEN_RANGE", "EQTOKEN_DISPLAY", "EQTOKEN_WALL", 
  "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT", "EQTOKEN_TOP_LEFT", 
  "EQTOKEN_SYNC", "EQTOKEN_LATENCY", "EQTOKEN_SWAPBARRIER", 
  "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME", "EQTOKEN_STRING", 
  "EQTOKEN_FLOAT", "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED", "'{'", "'}'", 
  "'['", "']'", "$accept", "file", "global", "globals", "globalStereo", 
  "connectionType", "server", "@1", "configs", "config", "@2", 
  "configFields", "configField", "nodes", "node", "otherNode", "@3", 
  "appNode", "@4", "nodeFields", "nodeField", "connections", "connection", 
  "@5", "connectionFields", "connectionField", "pipes", "pipe", "@6", 
  "pipeFields", "pipeField", "windows", "window", "@7", "windowFields", 
  "windowField", "windowHints", "windowHint", "hintFields", "hintField", 
  "stereo", "channels", "channel", "@8", "channelFields", "channelField", 
  "compounds", "compound", "@9", "compoundChildren", "compoundFields", 
  "compoundField", "@10", "@11", "@12", "compoundTasks", "compoundTask", 
  "compoundEyes", "compoundEye", "compoundFormats", "compoundFormat", 
  "wall", "swapBarrier", "@13", "swapBarrierFields", "swapBarrierField", 
  "outputFrame", "@14", "inputFrame", "@15", "frameFields", "frameField", 
  "viewport", "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   123,   125,
      91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    62,    63,    64,    64,    65,    65,    64,    64,    64,
      64,    64,    64,    66,    66,    66,    67,    69,    68,    70,
      70,    72,    71,    73,    73,    73,    74,    75,    75,    76,
      76,    78,    77,    80,    79,    81,    81,    81,    82,    83,
      83,    83,    85,    84,    86,    86,    86,    87,    87,    87,
      87,    88,    88,    90,    89,    91,    91,    91,    92,    92,
      93,    93,    95,    94,    96,    96,    96,    97,    97,    97,
      98,    98,    98,    99,   100,   100,   100,   101,   102,   102,
     102,   103,   103,   105,   104,   106,   106,   106,   107,   107,
     108,   108,   110,   109,   111,   111,   112,   112,   112,   113,
     113,   114,   113,   115,   113,   116,   113,   113,   113,   113,
     113,   113,   113,   117,   117,   117,   118,   118,   118,   119,
     119,   119,   120,   120,   120,   121,   121,   121,   122,   122,
     123,   125,   124,   126,   126,   126,   127,   129,   128,   131,
     130,   132,   132,   132,   133,   134,   135,   136,   136,   137,
     137,   138
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     0,     5,     1,
       2,     0,     7,     0,     1,     2,     2,     1,     2,     1,
       1,     0,     7,     0,     7,     0,     1,     2,     0,     0,
       1,     2,     0,     5,     0,     1,     2,     2,     2,     2,
       2,     1,     2,     0,     6,     0,     1,     2,     2,     2,
       1,     2,     0,     6,     0,     1,     2,     4,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     2,     1,     1,
       1,     1,     2,     0,     5,     0,     1,     2,     2,     2,
       1,     2,     0,     7,     0,     1,     0,     1,     2,     2,
       2,     0,     5,     0,     5,     0,     5,     2,     5,     1,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     0,     1,     2,     1,     1,
      21,     0,     5,     0,     1,     2,     2,     0,     5,     0,
       5,     0,     1,     2,     2,     6,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,    16,     7,   146,     8,   151,     9,    10,    11,    13,
      14,    15,    12,     1,     0,     2,     5,     0,    17,     3,
       6,     0,     0,     0,    19,    21,    18,    20,    23,     0,
       0,    24,    26,     0,     0,    25,     0,    27,    30,    29,
      33,    31,     0,    28,     0,    90,    39,    39,    92,    22,
      91,     0,    35,    40,    35,    96,    42,     0,    36,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    94,    97,   109,   110,   111,   112,    44,     0,
      37,     0,    51,     0,   100,    99,   101,   103,   105,     0,
     107,     0,     0,   131,   137,   139,    95,    96,    98,     0,
       0,     0,     0,     0,    45,    53,    34,    52,    32,   113,
     119,   125,   147,   149,     0,   148,   150,     0,     0,   133,
     141,   141,     0,    47,    48,    49,    50,    43,    46,    55,
     116,   117,   118,     0,   114,   122,   123,   124,     0,   120,
     128,   129,     0,   126,     0,     0,     0,     0,     0,   134,
       0,     0,   142,     0,    93,     0,     0,     0,    56,   102,
     115,   104,   121,   106,   127,     0,   108,     0,   136,   132,
     135,   144,   138,   143,   140,    59,    58,     0,    57,     0,
      60,     0,     0,    62,    54,    61,   145,     0,    64,     0,
       0,     0,     0,     0,    65,     0,    70,    68,    69,     0,
      66,     0,    81,     0,     0,     0,    71,    83,    63,    82,
       0,    74,    67,    72,    85,     0,     0,     0,    75,     0,
       0,     0,    86,     0,    78,    79,    80,    77,    73,    76,
      88,    89,    84,    87,     0,     0,     0,     0,     0,     0,
       0,   130
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     8,     9,    27,    22,    12,    25,    31,    33,    34,
      38,    40,    41,    46,    47,    48,    57,    49,    56,    67,
      68,    62,    63,    88,   113,   114,    91,    92,   139,   167,
     168,   189,   190,   198,   203,   204,   215,   216,   227,   228,
     237,   211,   212,   224,   231,   232,    54,    55,    65,   107,
      82,    83,   119,   120,   121,   143,   144,   148,   149,   152,
     153,    84,    85,   129,   158,   159,    86,   130,    87,   131,
     161,   162,   100,    14,   124,   125,   126
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -162
static const short yypact[] =
{
     168,   -31,    -8,   -20,   -14,   -14,   -20,   120,    49,    74,
     168,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,    30,  -162,  -162,     2,  -162,  -162,
    -162,   100,    63,     3,  -162,  -162,  -162,  -162,    75,   -14,
      20,  -162,  -162,    79,    84,  -162,   107,  -162,  -162,  -162,
    -162,  -162,    85,  -162,    12,  -162,   110,   110,  -162,  -162,
    -162,    89,   110,  -162,   110,   137,  -162,   138,  -162,  -162,
     138,   -20,   -20,    93,    97,   101,   118,   121,   102,   109,
     129,   136,   113,  -162,  -162,  -162,  -162,  -162,   103,   144,
    -162,     1,  -162,     8,  -162,  -162,  -162,  -162,  -162,   128,
    -162,   128,   140,  -162,  -162,  -162,   181,   137,  -162,    -8,
     -20,   -20,   -14,    16,  -162,  -162,  -162,  -162,  -162,   157,
     159,    73,  -162,  -162,   128,  -162,  -162,   128,   143,   182,
     183,   183,    65,  -162,  -162,  -162,  -162,  -162,  -162,   -19,
    -162,  -162,  -162,    17,  -162,  -162,  -162,  -162,    62,  -162,
    -162,  -162,    51,  -162,   128,   145,   128,   -20,    13,  -162,
     -20,    14,  -162,    15,  -162,   118,   -14,     6,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,   128,  -162,   128,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,   147,  -162,     9,
    -162,   148,   128,  -162,  -162,  -162,  -162,   149,    64,   164,
     154,   -20,   118,    60,  -162,   153,   197,  -162,  -162,   158,
    -162,    -4,  -162,   128,   160,     0,  -162,  -162,  -162,  -162,
     128,   199,  -162,  -162,     5,   128,   180,    -2,  -162,   -20,
     118,    -5,  -162,   161,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,   167,   165,   128,   128,   128,   163,
     162,  -162
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -162,  -162,    76,  -162,  -162,   111,  -162,  -162,  -162,   186,
    -162,  -162,   187,  -162,   184,  -162,  -162,  -162,  -162,   169,
     -39,   171,    19,  -162,  -162,   116,   156,    35,  -162,  -162,
      67,  -162,    42,  -162,  -162,    29,  -162,    21,  -162,    10,
    -162,  -162,    24,  -162,  -162,     7,   166,   -42,  -162,  -162,
     132,   -69,  -162,  -162,  -162,  -162,    98,  -162,    92,  -162,
      90,  -162,  -162,  -162,  -162,    86,  -162,  -162,  -162,  -162,
     112,   -68,  -161,    -6,   -98,  -162,    -3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      18,    16,    17,   127,   185,     1,     2,     3,     4,     5,
       6,     7,    60,   108,    32,    89,   226,   214,   209,    11,
     229,   187,    89,   165,   187,   166,   154,    10,    90,   155,
     229,    90,    43,    44,    13,    52,    42,   230,   157,   160,
     160,   208,   109,    15,   110,   111,   112,   230,   165,    23,
     166,   140,   141,   142,   242,   218,   175,   238,   177,   222,
     116,    29,    36,   108,    60,    94,    95,   118,   194,   241,
      39,    59,   179,   182,   184,   137,   200,   191,   169,   192,
     200,    69,   209,    69,    24,   201,    26,    71,    28,   201,
      72,   150,   151,   183,   197,   183,    73,    74,    75,   145,
     146,   147,   202,    30,   134,   135,   202,    76,    77,   136,
      78,    32,   173,   150,   151,   220,    79,    80,    81,    43,
      44,    35,   225,   171,   164,    39,   117,   233,   117,   109,
      52,   110,   111,   112,    61,    71,    52,    50,    72,    19,
      20,    21,    51,    58,    73,    74,    75,    66,   247,   248,
     249,   178,    89,    96,   181,    76,    77,    97,    78,    71,
     102,    98,    72,   186,    79,    80,    81,   103,    73,    74,
      75,     1,     2,     3,     4,     5,     6,     7,    99,    76,
      77,   101,    78,   122,   123,    15,   128,   104,    79,    80,
      81,   140,   141,   142,   105,   207,   145,   146,   147,   234,
     235,   236,   115,   156,    52,   193,   176,   157,   160,   196,
     199,   205,   206,   213,   214,   245,   217,   226,   221,    37,
     133,   251,   244,   240,   250,   246,    93,    45,    64,   138,
      53,   195,   210,    70,   188,   219,   223,   239,   243,   132,
     172,   170,   174,   163,   180,     0,     0,     0,   106
};

static const short yycheck[] =
{
       6,     4,     5,   101,   165,     3,     4,     5,     6,     7,
       8,     9,    54,    82,    11,    14,    18,    17,    22,    27,
      25,    15,    14,    42,    15,    44,   124,    58,    67,   127,
      25,    70,    12,    13,    54,    23,    39,    42,    25,    25,
      25,   202,    26,    57,    28,    29,    30,    42,    42,     0,
      44,    34,    35,    36,    59,    59,   154,    59,   156,    59,
      59,    59,    59,   132,   106,    71,    72,    59,    59,   230,
      50,    59,    59,    59,    59,    59,    16,   175,    61,   177,
      16,    62,    22,    64,    10,    25,    10,    22,    58,    25,
      25,    40,    41,   161,   192,   163,    31,    32,    33,    37,
      38,    39,    42,    27,   110,   111,    42,    42,    43,   112,
      45,    11,    61,    40,    41,   213,    51,    52,    53,    12,
      13,    58,   220,    61,    59,    50,    91,   225,    93,    26,
      23,    28,    29,    30,    24,    22,    23,    58,    25,    19,
      20,    21,    58,    58,    31,    32,    33,    58,   246,   247,
     248,   157,    14,    60,   160,    42,    43,    60,    45,    22,
      58,    60,    25,   166,    51,    52,    53,    58,    31,    32,
      33,     3,     4,     5,     6,     7,     8,     9,    60,    42,
      43,    60,    45,    55,    56,    57,    46,    58,    51,    52,
      53,    34,    35,    36,    58,   201,    37,    38,    39,    19,
      20,    21,    58,    60,    23,    58,    61,    25,    25,    61,
      61,    47,    58,    60,    17,    48,    58,    18,    58,    33,
     109,    59,    61,   229,    61,    60,    70,    40,    57,   113,
      46,   189,   203,    64,   167,   211,   215,   227,   231,   107,
     148,   143,   152,   131,   158,    -1,    -1,    -1,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    63,    64,
      58,    27,    67,    54,   135,    57,   138,   138,   135,    19,
      20,    21,    66,     0,    10,    68,    64,    65,    58,    59,
      64,    69,    11,    70,    71,    58,    59,    71,    72,    50,
      73,    74,   138,    12,    13,    74,    75,    76,    77,    79,
      58,    58,    23,    76,   108,   109,    80,    78,    58,    59,
     109,    24,    83,    84,    83,   110,    58,    81,    82,    84,
      81,    22,    25,    31,    32,    33,    42,    43,    45,    51,
      52,    53,   112,   113,   123,   124,   128,   130,    85,    14,
      82,    88,    89,    88,   135,   135,    60,    60,    60,    60,
     134,    60,    58,    58,    58,    58,   108,   111,   113,    26,
      28,    29,    30,    86,    87,    58,    59,    89,    59,   114,
     115,   116,    55,    56,   136,   137,   138,   136,    46,   125,
     129,   131,   112,    67,   135,   135,   138,    59,    87,    90,
      34,    35,    36,   117,   118,    37,    38,    39,   119,   120,
      40,    41,   121,   122,   136,   136,    60,    25,   126,   127,
      25,   132,   133,   132,    59,    42,    44,    91,    92,    61,
     118,    61,   120,    61,   122,   136,    61,   136,   135,    59,
     127,   135,    59,   133,    59,   134,   138,    15,    92,    93,
      94,   136,   136,    58,    59,    94,    61,   136,    95,    61,
      16,    25,    42,    96,    97,    47,    58,   135,   134,    22,
      97,   103,   104,    60,    17,    98,    99,    58,    59,   104,
     136,    58,    59,    99,   105,   136,    18,   100,   101,    25,
      42,   106,   107,   136,    19,    20,    21,   102,    59,   101,
     135,   134,    59,   107,    61,    48,    60,   136,   136,   136,
      61,    59
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

  case 13:

    { eqs::Global::instance()->setWindowIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_ON ); ;}
    break;

  case 14:

    { eqs::Global::instance()->setWindowIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_OFF ); ;}
    break;

  case 15:

    { eqs::Global::instance()->setWindowIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_AUTO );;}
    break;

  case 16:

    { yyval._connectionType = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 17:

    { server = loader->createServer(); ;}
    break;

  case 21:

    { config = loader->createConfig(); ;}
    break;

  case 22:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 26:

    { config->setLatency( yyvsp[0]._unsigned ); ;}
    break;

  case 31:

    { node = loader->createNode(); ;}
    break;

  case 32:

    { config->addNode( node ); node = NULL; ;}
    break;

  case 33:

    { node = loader->createNode(); ;}
    break;

  case 34:

    { config->addApplicationNode( node ); node = NULL; ;}
    break;

  case 39:

    { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             ;}
    break;

  case 42:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 43:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             ;}
    break;

  case 47:

    { connectionDescription->type = yyvsp[0]._connectionType; ;}
    break;

  case 48:

    { connectionDescription->hostname = yyvsp[0]._string; ;}
    break;

  case 49:

    { connectionDescription->launchCommand = yyvsp[0]._string; ;}
    break;

  case 50:

    { connectionDescription->launchTimeout = yyvsp[0]._unsigned; ;}
    break;

  case 53:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 54:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 58:

    { eqPipe->setDisplay( yyvsp[0]._unsigned ); ;}
    break;

  case 59:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], (int)yyvsp[0]._viewport[1],
                                                      (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
        ;}
    break;

  case 62:

    { window = loader->createWindow(); ;}
    break;

  case 63:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 68:

    { window->setName( yyvsp[0]._string ); ;}
    break;

  case 69:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], 
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                window->setViewport( eq::Viewport(yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); 
        ;}
    break;

  case 78:

    { window->setIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_ON ); ;}
    break;

  case 79:

    { window->setIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_OFF ); ;}
    break;

  case 80:

    { window->setIAttribute( 
                     eq::Window::IATTR_HINTS_STEREO, eq::STEREO_AUTO ); ;}
    break;

  case 83:

    { channel = loader->createChannel(); ;}
    break;

  case 84:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 88:

    { channel->setName( yyvsp[0]._string ); ;}
    break;

  case 89:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0],
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                channel->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3]));
        ;}
    break;

  case 92:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( compound )
                      compound->addChild( child );
                  else
                      config->addCompound( child );
                  compound = child;
              ;}
    break;

  case 93:

    { compound = compound->getParent(); ;}
    break;

  case 99:

    { compound->setName( yyvsp[0]._string ); ;}
    break;

  case 100:

    {
         eqs::Channel* channel = config->findChannel( yyvsp[0]._string );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             compound->setChannel( channel );
    ;}
    break;

  case 101:

    { compound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 103:

    { compound->setEyes( eqs::Compound::EYE_UNDEFINED ); ;}
    break;

  case 105:

    { compound->setFormats( eq::Frame::FORMAT_UNDEFINED );;}
    break;

  case 107:

    { compound->setViewport( eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3] )); ;}
    break;

  case 108:

    { compound->setRange( eq::Range( yyvsp[-2]._float, yyvsp[-1]._float )); ;}
    break;

  case 116:

    { compound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 117:

    { compound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 118:

    { compound->enableTask( eqs::Compound::TASK_READBACK ); ;}
    break;

  case 122:

    { compound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 123:

    { compound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 124:

    { compound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 128:

    { compound->enableFormat( eq::Frame::FORMAT_COLOR ); ;}
    break;

  case 129:

    { compound->enableFormat( eq::Frame::FORMAT_DEPTH ); ;}
    break;

  case 130:

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

  case 131:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 132:

    { 
            compound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 136:

    { swapBarrier->setName( yyvsp[0]._string ); ;}
    break;

  case 137:

    { frame = new eqs::Frame(); ;}
    break;

  case 138:

    { 
            compound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 139:

    { frame = new eqs::Frame(); ;}
    break;

  case 140:

    { 
            compound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 144:

    { frame->setName( yyvsp[0]._string ); ;}
    break;

  case 145:

    { 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;}
    break;

  case 146:

    {
         stringBuf = yytext;
         yyval._string = stringBuf.c_str(); 
     ;}
    break;

  case 147:

    { yyval._float = atof( yytext ); ;}
    break;

  case 148:

    { yyval._float = yyvsp[0]._int; ;}
    break;

  case 149:

    { yyval._int = atoi( yytext ); ;}
    break;

  case 150:

    { yyval._int = yyvsp[0]._unsigned; ;}
    break;

  case 151:

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

