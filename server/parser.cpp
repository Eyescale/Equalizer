/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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

/* Substitute the variable and function names.  */
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
     EQTOKEN_CONNECTION_SATTR_HOSTNAME = 259,
     EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND = 260,
     EQTOKEN_CONNECTION_IATTR_TYPE = 261,
     EQTOKEN_CONNECTION_IATTR_TCPIP_PORT = 262,
     EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT = 263,
     EQTOKEN_CONFIG_FATTR_EYE_BASE = 264,
     EQTOKEN_WINDOW_IATTR_HINTS_STEREO = 265,
     EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER = 266,
     EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN = 267,
     EQTOKEN_WINDOW_IATTR_HINTS_DECORATION = 268,
     EQTOKEN_WINDOW_IATTR_PLANES_COLOR = 269,
     EQTOKEN_WINDOW_IATTR_PLANES_ALPHA = 270,
     EQTOKEN_WINDOW_IATTR_PLANES_DEPTH = 271,
     EQTOKEN_WINDOW_IATTR_PLANES_STENCIL = 272,
     EQTOKEN_SERVER = 273,
     EQTOKEN_CONFIG = 274,
     EQTOKEN_APPNODE = 275,
     EQTOKEN_NODE = 276,
     EQTOKEN_PIPE = 277,
     EQTOKEN_WINDOW = 278,
     EQTOKEN_ATTRIBUTES = 279,
     EQTOKEN_HINTS = 280,
     EQTOKEN_STEREO = 281,
     EQTOKEN_DOUBLEBUFFER = 282,
     EQTOKEN_FULLSCREEN = 283,
     EQTOKEN_DECORATION = 284,
     EQTOKEN_PLANES = 285,
     EQTOKEN_COLOR = 286,
     EQTOKEN_ALPHA = 287,
     EQTOKEN_DEPTH = 288,
     EQTOKEN_STENCIL = 289,
     EQTOKEN_ON = 290,
     EQTOKEN_OFF = 291,
     EQTOKEN_AUTO = 292,
     EQTOKEN_CHANNEL = 293,
     EQTOKEN_COMPOUND = 294,
     EQTOKEN_CONNECTION = 295,
     EQTOKEN_NAME = 296,
     EQTOKEN_TYPE = 297,
     EQTOKEN_TCPIP = 298,
     EQTOKEN_HOSTNAME = 299,
     EQTOKEN_COMMAND = 300,
     EQTOKEN_TIMEOUT = 301,
     EQTOKEN_TASK = 302,
     EQTOKEN_EYE = 303,
     EQTOKEN_EYE_BASE = 304,
     EQTOKEN_BUFFER = 305,
     EQTOKEN_CLEAR = 306,
     EQTOKEN_DRAW = 307,
     EQTOKEN_ASSEMBLE = 308,
     EQTOKEN_READBACK = 309,
     EQTOKEN_CYCLOP = 310,
     EQTOKEN_LEFT = 311,
     EQTOKEN_RIGHT = 312,
     EQTOKEN_VIEWPORT = 313,
     EQTOKEN_RANGE = 314,
     EQTOKEN_DISPLAY = 315,
     EQTOKEN_SCREEN = 316,
     EQTOKEN_WALL = 317,
     EQTOKEN_BOTTOM_LEFT = 318,
     EQTOKEN_BOTTOM_RIGHT = 319,
     EQTOKEN_TOP_LEFT = 320,
     EQTOKEN_SYNC = 321,
     EQTOKEN_LATENCY = 322,
     EQTOKEN_SWAPBARRIER = 323,
     EQTOKEN_OUTPUTFRAME = 324,
     EQTOKEN_INPUTFRAME = 325,
     EQTOKEN_STRING = 326,
     EQTOKEN_FLOAT = 327,
     EQTOKEN_INTEGER = 328,
     EQTOKEN_UNSIGNED = 329
   };
#endif
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_SATTR_HOSTNAME 259
#define EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND 260
#define EQTOKEN_CONNECTION_IATTR_TYPE 261
#define EQTOKEN_CONNECTION_IATTR_TCPIP_PORT 262
#define EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT 263
#define EQTOKEN_CONFIG_FATTR_EYE_BASE 264
#define EQTOKEN_WINDOW_IATTR_HINTS_STEREO 265
#define EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER 266
#define EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN 267
#define EQTOKEN_WINDOW_IATTR_HINTS_DECORATION 268
#define EQTOKEN_WINDOW_IATTR_PLANES_COLOR 269
#define EQTOKEN_WINDOW_IATTR_PLANES_ALPHA 270
#define EQTOKEN_WINDOW_IATTR_PLANES_DEPTH 271
#define EQTOKEN_WINDOW_IATTR_PLANES_STENCIL 272
#define EQTOKEN_SERVER 273
#define EQTOKEN_CONFIG 274
#define EQTOKEN_APPNODE 275
#define EQTOKEN_NODE 276
#define EQTOKEN_PIPE 277
#define EQTOKEN_WINDOW 278
#define EQTOKEN_ATTRIBUTES 279
#define EQTOKEN_HINTS 280
#define EQTOKEN_STEREO 281
#define EQTOKEN_DOUBLEBUFFER 282
#define EQTOKEN_FULLSCREEN 283
#define EQTOKEN_DECORATION 284
#define EQTOKEN_PLANES 285
#define EQTOKEN_COLOR 286
#define EQTOKEN_ALPHA 287
#define EQTOKEN_DEPTH 288
#define EQTOKEN_STENCIL 289
#define EQTOKEN_ON 290
#define EQTOKEN_OFF 291
#define EQTOKEN_AUTO 292
#define EQTOKEN_CHANNEL 293
#define EQTOKEN_COMPOUND 294
#define EQTOKEN_CONNECTION 295
#define EQTOKEN_NAME 296
#define EQTOKEN_TYPE 297
#define EQTOKEN_TCPIP 298
#define EQTOKEN_HOSTNAME 299
#define EQTOKEN_COMMAND 300
#define EQTOKEN_TIMEOUT 301
#define EQTOKEN_TASK 302
#define EQTOKEN_EYE 303
#define EQTOKEN_EYE_BASE 304
#define EQTOKEN_BUFFER 305
#define EQTOKEN_CLEAR 306
#define EQTOKEN_DRAW 307
#define EQTOKEN_ASSEMBLE 308
#define EQTOKEN_READBACK 309
#define EQTOKEN_CYCLOP 310
#define EQTOKEN_LEFT 311
#define EQTOKEN_RIGHT 312
#define EQTOKEN_VIEWPORT 313
#define EQTOKEN_RANGE 314
#define EQTOKEN_DISPLAY 315
#define EQTOKEN_SCREEN 316
#define EQTOKEN_WALL 317
#define EQTOKEN_BOTTOM_LEFT 318
#define EQTOKEN_BOTTOM_RIGHT 319
#define EQTOKEN_TOP_LEFT 320
#define EQTOKEN_SYNC 321
#define EQTOKEN_LATENCY 322
#define EQTOKEN_SWAPBARRIER 323
#define EQTOKEN_OUTPUTFRAME 324
#define EQTOKEN_INPUTFRAME 325
#define EQTOKEN_STRING 326
#define EQTOKEN_FLOAT 327
#define EQTOKEN_INTEGER 328
#define EQTOKEN_UNSIGNED 329




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
        static eqs::Compound*    eqCompound; // avoid name clash on Darwin
        static eqs::SwapBarrier* swapBarrier;
        static eqs::Frame*       frame;
        static eqBase::RefPtr<eqNet::ConnectionDescription> 
            connectionDescription;
        static uint32_t          flags;
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
/* Line 190 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
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
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   337

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  81
/* YYNRULES -- Number of rules. */
#define YYNRULES  178
/* YYNRULES -- Number of states. */
#define YYNSTATES  304

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

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
       2,    77,     2,    78,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    75,     2,    76,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    38,    41,    44,    47,    50,    53,
      56,    59,    61,    62,    68,    70,    73,    74,    82,    83,
      85,    88,    91,    96,    97,    99,   102,   105,   107,   110,
     112,   114,   115,   123,   124,   132,   133,   135,   138,   139,
     140,   142,   145,   146,   152,   153,   155,   158,   161,   164,
     167,   170,   172,   175,   176,   183,   184,   186,   189,   192,
     195,   198,   200,   203,   204,   211,   212,   214,   217,   222,
     225,   228,   229,   231,   234,   239,   244,   245,   247,   250,
     253,   256,   259,   262,   263,   265,   268,   271,   274,   277,
     280,   282,   285,   286,   292,   293,   295,   298,   301,   304,
     306,   309,   310,   318,   319,   321,   322,   324,   327,   330,
     333,   334,   340,   341,   347,   348,   354,   357,   363,   365,
     367,   369,   371,   372,   374,   377,   379,   381,   383,   385,
     386,   388,   391,   393,   395,   397,   398,   400,   403,   405,
     407,   429,   430,   436,   437,   439,   442,   445,   446,   452,
     453,   459,   460,   462,   465,   468,   471,   472,   478,   485,
     487,   489,   491,   493,   495,   497,   499,   501,   503
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
      80,     0,    -1,    81,    84,    -1,     3,    75,    82,    76,
      -1,    -1,    81,    -1,    82,    81,    -1,     4,   156,    -1,
       5,   156,    -1,     6,    83,    -1,     7,   159,    -1,     8,
     159,    -1,    10,   155,    -1,    11,   155,    -1,    12,   155,
      -1,    13,   155,    -1,    14,   155,    -1,    15,   155,    -1,
      16,   155,    -1,    17,   155,    -1,     9,   157,    -1,    43,
      -1,    -1,    18,    75,    85,    86,    76,    -1,    87,    -1,
      86,    87,    -1,    -1,    19,    75,    88,    89,    93,   127,
      76,    -1,    -1,    90,    -1,    89,    90,    -1,    67,   159,
      -1,    24,    75,    91,    76,    -1,    -1,    92,    -1,    91,
      92,    -1,    49,   157,    -1,    94,    -1,    93,    94,    -1,
      97,    -1,    95,    -1,    -1,    21,    75,    96,   101,    99,
     106,    76,    -1,    -1,    20,    75,    98,   101,    99,   106,
      76,    -1,    -1,   100,    -1,    99,   100,    -1,    -1,    -1,
     102,    -1,   101,   102,    -1,    -1,    40,    75,   103,   104,
      76,    -1,    -1,   105,    -1,   104,   105,    -1,    42,    83,
      -1,    44,   156,    -1,    45,   156,    -1,    46,   159,    -1,
     107,    -1,   106,   107,    -1,    -1,    22,    75,   108,   109,
     111,    76,    -1,    -1,   110,    -1,   109,   110,    -1,    60,
     159,    -1,    61,   159,    -1,    58,   154,    -1,   112,    -1,
     111,   112,    -1,    -1,    23,    75,   113,   114,   122,    76,
      -1,    -1,   115,    -1,   114,   115,    -1,    24,    75,   116,
      76,    -1,    41,   156,    -1,    58,   154,    -1,    -1,   117,
      -1,   116,   117,    -1,    25,    75,   118,    76,    -1,    30,
      75,   120,    76,    -1,    -1,   119,    -1,   118,   119,    -1,
      26,   155,    -1,    27,   155,    -1,    28,   155,    -1,    29,
     155,    -1,    -1,   121,    -1,   120,   121,    -1,    31,   155,
      -1,    32,   155,    -1,    33,   155,    -1,    34,   155,    -1,
     123,    -1,   122,   123,    -1,    -1,    38,    75,   124,   125,
      76,    -1,    -1,   126,    -1,   125,   126,    -1,    41,   156,
      -1,    58,   154,    -1,   128,    -1,   127,   128,    -1,    -1,
      39,    75,   129,   131,   130,   131,    76,    -1,    -1,   127,
      -1,    -1,   132,    -1,   131,   132,    -1,    41,   156,    -1,
      38,   156,    -1,    -1,    47,    77,   133,   136,    78,    -1,
      -1,    48,    77,   134,   138,    78,    -1,    -1,    50,    77,
     135,   140,    78,    -1,    58,   154,    -1,    59,    77,   157,
     157,    78,    -1,   142,    -1,   143,    -1,   147,    -1,   149,
      -1,    -1,   137,    -1,   136,   137,    -1,    51,    -1,    52,
      -1,    53,    -1,    54,    -1,    -1,   139,    -1,   138,   139,
      -1,    55,    -1,    56,    -1,    57,    -1,    -1,   141,    -1,
     140,   141,    -1,    31,    -1,    33,    -1,    62,    75,    63,
      77,   157,   157,   157,    78,    64,    77,   157,   157,   157,
      78,    65,    77,   157,   157,   157,    78,    76,    -1,    -1,
      68,    75,   144,   145,    76,    -1,    -1,   146,    -1,   145,
     146,    -1,    41,   156,    -1,    -1,    69,    75,   148,   151,
      76,    -1,    -1,    70,    75,   150,   151,    76,    -1,    -1,
     152,    -1,   151,   152,    -1,    41,   156,    -1,    58,   154,
      -1,    -1,    50,    77,   153,   140,    78,    -1,    77,   157,
     157,   157,   157,    78,    -1,    35,    -1,    36,    -1,    37,
      -1,   158,    -1,    71,    -1,    72,    -1,   158,    -1,    73,
      -1,   159,    -1,    74,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   143,   143,   145,   146,   149,   149,   152,   157,   162,
     167,   172,   177,   182,   187,   192,   197,   202,   207,   212,
     217,   223,   225,   225,   228,   228,   229,   229,   232,   232,
     232,   234,   235,   236,   236,   236,   238,   241,   241,   242,
     242,   243,   243,   247,   247,   251,   251,   251,   252,   255,
     259,   259,   261,   260,   267,   267,   268,   270,   271,   272,
     273,   276,   276,   277,   277,   280,   280,   280,   282,   283,
     284,   290,   290,   291,   291,   294,   294,   294,   296,   298,
     299,   307,   307,   307,   309,   310,   311,   311,   311,   313,
     315,   317,   319,   321,   321,   321,   323,   325,   327,   329,
     332,   332,   333,   333,   336,   337,   337,   339,   340,   350,
     350,   352,   351,   365,   365,   367,   367,   368,   370,   371,
     379,   379,   381,   381,   383,   383,   385,   387,   389,   390,
     391,   392,   394,   394,   394,   396,   397,   398,   399,   401,
     401,   401,   403,   404,   405,   407,   407,   407,   409,   410,
     412,   433,   433,   439,   439,   440,   442,   444,   444,   450,
     450,   456,   456,   456,   458,   459,   461,   461,   464,   473,
     474,   475,   476,   478,   483,   484,   486,   487,   488
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQTOKEN_GLOBAL",
  "EQTOKEN_CONNECTION_SATTR_HOSTNAME",
  "EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND",
  "EQTOKEN_CONNECTION_IATTR_TYPE", "EQTOKEN_CONNECTION_IATTR_TCPIP_PORT",
  "EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT",
  "EQTOKEN_CONFIG_FATTR_EYE_BASE", "EQTOKEN_WINDOW_IATTR_HINTS_STEREO",
  "EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER",
  "EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN",
  "EQTOKEN_WINDOW_IATTR_HINTS_DECORATION",
  "EQTOKEN_WINDOW_IATTR_PLANES_COLOR", "EQTOKEN_WINDOW_IATTR_PLANES_ALPHA",
  "EQTOKEN_WINDOW_IATTR_PLANES_DEPTH",
  "EQTOKEN_WINDOW_IATTR_PLANES_STENCIL", "EQTOKEN_SERVER",
  "EQTOKEN_CONFIG", "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE",
  "EQTOKEN_WINDOW", "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINTS",
  "EQTOKEN_STEREO", "EQTOKEN_DOUBLEBUFFER", "EQTOKEN_FULLSCREEN",
  "EQTOKEN_DECORATION", "EQTOKEN_PLANES", "EQTOKEN_COLOR", "EQTOKEN_ALPHA",
  "EQTOKEN_DEPTH", "EQTOKEN_STENCIL", "EQTOKEN_ON", "EQTOKEN_OFF",
  "EQTOKEN_AUTO", "EQTOKEN_CHANNEL", "EQTOKEN_COMPOUND",
  "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE", "EQTOKEN_TCPIP",
  "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND", "EQTOKEN_TIMEOUT", "EQTOKEN_TASK",
  "EQTOKEN_EYE", "EQTOKEN_EYE_BASE", "EQTOKEN_BUFFER", "EQTOKEN_CLEAR",
  "EQTOKEN_DRAW", "EQTOKEN_ASSEMBLE", "EQTOKEN_READBACK", "EQTOKEN_CYCLOP",
  "EQTOKEN_LEFT", "EQTOKEN_RIGHT", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE",
  "EQTOKEN_DISPLAY", "EQTOKEN_SCREEN", "EQTOKEN_WALL",
  "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT", "EQTOKEN_TOP_LEFT",
  "EQTOKEN_SYNC", "EQTOKEN_LATENCY", "EQTOKEN_SWAPBARRIER",
  "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME", "EQTOKEN_STRING",
  "EQTOKEN_FLOAT", "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED", "'{'", "'}'",
  "'['", "']'", "$accept", "file", "global", "globals", "connectionType",
  "server", "@1", "configs", "config", "@2", "configFields", "configField",
  "configAttributes", "configAttribute", "nodes", "node", "otherNode",
  "@3", "appNode", "@4", "nodeFields", "nodeField", "connections",
  "connection", "@5", "connectionFields", "connectionField", "pipes",
  "pipe", "@6", "pipeFields", "pipeField", "windows", "window", "@7",
  "windowFields", "windowField", "windowAttributes", "windowAttribute",
  "windowHints", "windowHint", "windowPlanes", "windowPlane", "channels",
  "channel", "@8", "channelFields", "channelField", "compounds",
  "compound", "@9", "compoundChildren", "compoundFields", "compoundField",
  "@10", "@11", "@12", "compoundTasks", "compoundTask", "compoundEyes",
  "compoundEye", "buffers", "buffer", "wall", "swapBarrier", "@13",
  "swapBarrierFields", "swapBarrierField", "outputFrame", "@14",
  "inputFrame", "@15", "frameFields", "frameField", "@16", "viewport",
  "IATTR", "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   123,   125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    79,    80,    81,    81,    82,    82,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    83,    85,    84,    86,    86,    88,    87,    89,    89,
      89,    90,    90,    91,    91,    91,    92,    93,    93,    94,
      94,    96,    95,    98,    97,    99,    99,    99,   100,   101,
     101,   101,   103,   102,   104,   104,   104,   105,   105,   105,
     105,   106,   106,   108,   107,   109,   109,   109,   110,   110,
     110,   111,   111,   113,   112,   114,   114,   114,   115,   115,
     115,   116,   116,   116,   117,   117,   118,   118,   118,   119,
     119,   119,   119,   120,   120,   120,   121,   121,   121,   121,
     122,   122,   124,   123,   125,   125,   125,   126,   126,   127,
     127,   129,   128,   130,   130,   131,   131,   131,   132,   132,
     133,   132,   134,   132,   135,   132,   132,   132,   132,   132,
     132,   132,   136,   136,   136,   137,   137,   137,   137,   138,
     138,   138,   139,   139,   139,   140,   140,   140,   141,   141,
     142,   144,   143,   145,   145,   145,   146,   148,   147,   150,
     149,   151,   151,   151,   152,   152,   153,   152,   154,   155,
     155,   155,   155,   156,   157,   157,   158,   158,   159
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     0,     5,     1,     2,     0,     7,     0,     1,
       2,     2,     4,     0,     1,     2,     2,     1,     2,     1,
       1,     0,     7,     0,     7,     0,     1,     2,     0,     0,
       1,     2,     0,     5,     0,     1,     2,     2,     2,     2,
       2,     1,     2,     0,     6,     0,     1,     2,     2,     2,
       2,     1,     2,     0,     6,     0,     1,     2,     4,     2,
       2,     0,     1,     2,     4,     4,     0,     1,     2,     2,
       2,     2,     2,     0,     1,     2,     2,     2,     2,     2,
       1,     2,     0,     5,     0,     1,     2,     2,     2,     1,
       2,     0,     7,     0,     1,     0,     1,     2,     2,     2,
       0,     5,     0,     5,     0,     5,     2,     5,     1,     1,
       1,     1,     0,     1,     2,     1,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     0,     1,     2,     1,     1,
      21,     0,     5,     0,     1,     2,     2,     0,     5,     0,
       5,     0,     1,     2,     2,     2,     0,     5,     6,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,   173,
       7,     8,    21,     9,   178,    10,    11,   174,   176,    20,
     175,   177,   169,   170,   171,    12,   172,    13,    14,    15,
      16,    17,    18,    19,     1,     0,     2,     5,     0,    22,
       3,     6,     0,     0,     0,    24,    26,    23,    25,    28,
       0,     0,     0,    29,    33,    31,     0,     0,    30,     0,
      37,    40,    39,     0,     0,    34,    43,    41,     0,    38,
       0,   109,    36,    32,    35,    49,    49,   111,    27,   110,
       0,    45,    50,    45,   115,    52,     0,    46,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   113,   116,   128,   129,   130,   131,    54,     0,    47,
       0,    61,     0,   119,   118,   120,   122,   124,     0,   126,
       0,     0,   151,   157,   159,   114,   115,   117,     0,     0,
       0,     0,     0,    55,    63,    44,    62,    42,   132,   139,
     145,     0,     0,     0,   153,   161,   161,     0,    57,    58,
      59,    60,    53,    56,    65,   135,   136,   137,   138,     0,
     133,   142,   143,   144,     0,   140,   148,   149,     0,   146,
       0,     0,     0,     0,     0,   154,     0,     0,     0,     0,
     162,     0,   112,     0,     0,     0,     0,    66,   121,   134,
     123,   141,   125,   147,     0,   127,     0,   156,   152,   155,
     164,   166,   165,   158,   163,   160,    70,    68,    69,     0,
      67,     0,    71,     0,     0,   145,    73,    64,    72,   168,
       0,     0,    75,     0,   167,     0,     0,     0,     0,    76,
       0,    81,    79,    80,     0,    77,     0,   100,     0,     0,
       0,     0,    82,   102,    74,   101,     0,    86,    93,    78,
      83,   104,     0,     0,     0,     0,     0,     0,    87,     0,
       0,     0,     0,     0,    94,     0,     0,     0,   105,     0,
      89,    90,    91,    92,    84,    88,    96,    97,    98,    99,
      85,    95,   107,   108,   103,   106,     0,     0,     0,     0,
       0,     0,     0,   150
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    16,    17,    48,    23,    46,    52,    54,    55,    59,
      62,    63,    74,    75,    69,    70,    71,    86,    72,    85,
      96,    97,    91,    92,   117,   142,   143,   120,   121,   164,
     196,   197,   221,   222,   232,   238,   239,   251,   252,   267,
     268,   273,   274,   246,   247,   261,   277,   278,    80,    81,
      94,   136,   111,   112,   148,   149,   150,   169,   170,   174,
     175,   178,   179,   113,   114,   154,   184,   185,   115,   155,
     116,   156,   189,   190,   225,   129,    35,    20,    29,    30,
      31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -169
static const short int yypact[] =
{
     274,   -27,   -14,   -14,    89,   -13,   -13,   103,    46,    46,
      46,    46,    46,    46,    46,    46,   150,    50,   274,  -169,
    -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,  -169,    14,  -169,  -169,    23,  -169,
    -169,  -169,   140,    90,     2,  -169,  -169,  -169,  -169,    19,
      95,   -13,    35,  -169,   130,  -169,   107,   111,  -169,   142,
    -169,  -169,  -169,   103,    73,  -169,  -169,  -169,   128,  -169,
      24,  -169,  -169,  -169,  -169,   155,   155,  -169,  -169,  -169,
     134,   155,  -169,   155,   182,  -169,   188,  -169,  -169,   188,
     -14,   -14,   139,   144,   145,   151,   154,   163,   167,   168,
     174,   149,  -169,  -169,  -169,  -169,  -169,   160,   179,  -169,
       0,  -169,     1,  -169,  -169,  -169,  -169,  -169,   103,  -169,
     103,   197,  -169,  -169,  -169,   222,   182,  -169,    89,   -14,
     -14,   -13,   109,  -169,  -169,  -169,  -169,  -169,   161,   241,
     112,   103,   103,   189,   230,    21,    21,    65,  -169,  -169,
    -169,  -169,  -169,  -169,   234,  -169,  -169,  -169,  -169,   115,
    -169,  -169,  -169,  -169,   116,  -169,  -169,  -169,    13,  -169,
     103,   215,   103,   -14,     8,  -169,   -14,   223,   151,    88,
    -169,    98,  -169,   151,   -13,   -13,    70,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,   103,  -169,   103,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,   224,
    -169,    18,  -169,   225,   103,   112,  -169,  -169,  -169,  -169,
     226,    27,    68,   237,  -169,   227,   -14,   151,   120,  -169,
     228,   127,  -169,  -169,   231,  -169,     9,  -169,   103,   232,
     233,    20,  -169,  -169,  -169,  -169,   103,   198,   214,  -169,
    -169,    63,   103,    46,    46,    46,    46,    25,  -169,    46,
      46,    46,    46,    41,  -169,   -14,   151,    49,  -169,   235,
    -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,  -169,
    -169,  -169,  -169,  -169,  -169,  -169,   244,   238,   103,   103,
     103,   236,   240,  -169
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -169,  -169,    62,  -169,   172,  -169,  -169,  -169,   257,  -169,
    -169,   250,  -169,   243,  -169,   249,  -169,  -169,  -169,  -169,
     229,    15,   239,   146,  -169,  -169,   177,   221,    69,  -169,
    -169,   125,  -169,   102,  -169,  -169,    86,  -169,    75,  -169,
      60,  -169,    55,  -169,    83,  -169,  -169,    53,   220,   -70,
    -169,  -169,   196,   -87,  -169,  -169,  -169,  -169,   164,  -169,
     162,   110,  -136,  -169,  -169,  -169,  -169,   153,  -169,  -169,
    -169,  -169,   178,    64,  -169,  -168,     4,    -3,   -64,    -7,
       6
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
      21,    36,    36,    36,    36,    36,    36,    36,    36,    82,
      89,    25,    26,    37,    38,    39,    40,    41,    42,    43,
     212,    53,   118,   118,   137,   216,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,   219,   203,    60,   176,   249,   177,   244,    18,   183,
     250,   263,   264,   265,   266,    66,    67,    19,   176,    60,
     177,    24,   186,    78,   151,    89,   152,    65,    45,   243,
     137,   187,   269,   270,   271,   272,   145,   147,    57,   188,
      47,    32,    33,    34,   208,   254,    61,   180,   181,    49,
     275,   202,   235,   219,   227,   203,   259,   123,   124,    50,
      88,   284,    61,   100,   275,   234,   101,   276,   293,   236,
      51,   119,   102,   103,   119,   104,   204,   290,   206,    28,
      24,   276,    73,   105,   106,   294,   237,   107,   193,   186,
     194,   195,    22,   108,   109,   110,   159,   160,   187,   186,
     223,   192,   224,   176,   235,   177,   188,   161,   187,    83,
      44,   138,   249,   139,   140,   141,   188,   250,   244,    53,
     230,   236,    66,    67,   213,    56,   165,   166,   167,   168,
      64,   171,   172,   173,   215,    27,    28,    24,   237,    73,
     207,    78,    76,   210,   256,   162,    77,   100,    78,   146,
     101,   146,   262,   198,   200,    90,   102,   103,   279,   104,
     217,   218,   138,    87,   139,   140,   141,   105,   106,    95,
     118,   107,   165,   166,   167,   168,   125,   108,   109,   110,
     100,   126,   127,   101,   263,   264,   265,   266,   128,   102,
     103,   130,   104,   242,   299,   300,   301,    98,   131,    98,
     105,   106,   132,   133,   107,   269,   270,   271,   272,   134,
     108,   109,   110,   214,   144,   214,    36,    36,    36,    36,
     153,    78,    36,    36,    36,    36,   182,   280,   281,   282,
     283,   183,   292,   286,   287,   288,   289,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,   193,   205,   194,   195,   171,   172,   173,   226,
     211,   240,   241,   229,   233,   248,   253,   257,   258,   297,
     158,    58,    68,   296,   302,   298,   303,    84,    79,   163,
     122,   220,    99,   228,   245,    93,   260,   285,   291,   255,
     295,   135,   157,   199,   191,   231,   201,   209
};

static const unsigned short int yycheck[] =
{
       3,     8,     9,    10,    11,    12,    13,    14,    15,    73,
      80,     5,     6,     9,    10,    11,    12,    13,    14,    15,
     188,    19,    22,    22,   111,   193,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    23,   178,    24,    31,    25,    33,    38,    75,    41,
      30,    26,    27,    28,    29,    20,    21,    71,    31,    24,
      33,    74,    41,    39,   128,   135,   130,    61,    18,   237,
     157,    50,    31,    32,    33,    34,    76,    76,    76,    58,
      18,    35,    36,    37,    76,    76,    67,   151,   152,    75,
      41,    78,    24,    23,    76,   231,    76,   100,   101,    76,
      76,    76,    67,    38,    41,    78,    41,    58,   276,    41,
      48,    96,    47,    48,    99,    50,   180,    76,   182,    73,
      74,    58,    49,    58,    59,    76,    58,    62,    58,    41,
      60,    61,    43,    68,    69,    70,   139,   140,    50,    41,
     204,    76,   206,    31,    24,    33,    58,   141,    50,    76,
       0,    42,    25,    44,    45,    46,    58,    30,    38,    19,
     224,    41,    20,    21,    76,    75,    51,    52,    53,    54,
      75,    55,    56,    57,    76,    72,    73,    74,    58,    49,
     183,    39,    75,   186,   248,    76,    75,    38,    39,   120,
      41,   122,   256,    78,    78,    40,    47,    48,   262,    50,
     194,   195,    42,    75,    44,    45,    46,    58,    59,    75,
      22,    62,    51,    52,    53,    54,    77,    68,    69,    70,
      38,    77,    77,    41,    26,    27,    28,    29,    77,    47,
      48,    77,    50,   236,   298,   299,   300,    91,    75,    93,
      58,    59,    75,    75,    62,    31,    32,    33,    34,    75,
      68,    69,    70,   189,    75,   191,   263,   264,   265,   266,
      63,    39,   269,   270,   271,   272,    77,   263,   264,   265,
     266,    41,   275,   269,   270,   271,   272,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    58,    78,    60,    61,    55,    56,    57,    75,
      77,    64,    75,    78,    78,    77,    75,    75,    75,    65,
     138,    54,    62,    78,    78,    77,    76,    74,    69,   142,
      99,   196,    93,   221,   238,    86,   251,   267,   273,   246,
     277,   111,   136,   169,   156,   225,   174,   184
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    80,    81,    75,    71,
     156,   156,    43,    83,    74,   159,   159,    72,    73,   157,
     158,   159,    35,    36,    37,   155,   158,   155,   155,   155,
     155,   155,   155,   155,     0,    18,    84,    81,    82,    75,
      76,    81,    85,    19,    86,    87,    75,    76,    87,    88,
      24,    67,    89,    90,    75,   159,    20,    21,    90,    93,
      94,    95,    97,    49,    91,    92,    75,    75,    39,    94,
     127,   128,   157,    76,    92,    98,    96,    75,    76,   128,
      40,   101,   102,   101,   129,    75,    99,   100,   102,    99,
      38,    41,    47,    48,    50,    58,    59,    62,    68,    69,
      70,   131,   132,   142,   143,   147,   149,   103,    22,   100,
     106,   107,   106,   156,   156,    77,    77,    77,    77,   154,
      77,    75,    75,    75,    75,   127,   130,   132,    42,    44,
      45,    46,   104,   105,    75,    76,   107,    76,   133,   134,
     135,   157,   157,    63,   144,   148,   150,   131,    83,   156,
     156,   159,    76,   105,   108,    51,    52,    53,    54,   136,
     137,    55,    56,    57,   138,   139,    31,    33,   140,   141,
     157,   157,    77,    41,   145,   146,    41,    50,    58,   151,
     152,   151,    76,    58,    60,    61,   109,   110,    78,   137,
      78,   139,    78,   141,   157,    78,   157,   156,    76,   146,
     156,    77,   154,    76,   152,    76,   154,   159,   159,    23,
     110,   111,   112,   157,   157,   153,    75,    76,   112,    78,
     157,   140,   113,    78,    78,    24,    41,    58,   114,   115,
      64,    75,   156,   154,    38,   115,   122,   123,    77,    25,
      30,   116,   117,    75,    76,   123,   157,    75,    75,    76,
     117,   124,   157,    26,    27,    28,    29,   118,   119,    31,
      32,    33,    34,   120,   121,    41,    58,   125,   126,   157,
     155,   155,   155,   155,    76,   119,   155,   155,   155,   155,
      76,   121,   156,   154,    76,   126,    78,    65,    77,   157,
     157,   157,    78,    76
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
#define YYERROR		goto yyerrorlab


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


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
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
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
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
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
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
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

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


  yyvsp[0] = yylval;

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
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
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
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

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
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, (yyvsp[0]._string) );
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, (yyvsp[0]._string) );
     ;}
    break;

  case 9:

    { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, (yyvsp[0]._connectionType) ); 
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, (yyvsp[0]._unsigned) );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, (yyvsp[0]._unsigned) );
     ;}
    break;

  case 12:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_STEREO, (yyvsp[0]._int) );
     ;}
    break;

  case 13:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DOUBLEBUFFER, (yyvsp[0]._int) );
     ;}
    break;

  case 14:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_FULLSCREEN, (yyvsp[0]._int) );
     ;}
    break;

  case 15:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DECORATION, (yyvsp[0]._int) );
     ;}
    break;

  case 16:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, (yyvsp[0]._int) );
     ;}
    break;

  case 17:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, (yyvsp[0]._int) );
     ;}
    break;

  case 18:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, (yyvsp[0]._int) );
     ;}
    break;

  case 19:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, (yyvsp[0]._int) );
     ;}
    break;

  case 20:

    {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, (yyvsp[0]._float) );
     ;}
    break;

  case 21:

    { (yyval._connectionType) = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 22:

    { server = loader->createServer(); ;}
    break;

  case 26:

    { config = loader->createConfig(); ;}
    break;

  case 27:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 31:

    { config->setLatency( (yyvsp[0]._unsigned) ); ;}
    break;

  case 36:

    { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, (yyvsp[0]._float) ); ;}
    break;

  case 41:

    { node = loader->createNode(); ;}
    break;

  case 42:

    { config->addNode( node ); node = NULL; ;}
    break;

  case 43:

    { node = loader->createNode(); ;}
    break;

  case 44:

    { config->addApplicationNode( node ); node = NULL; ;}
    break;

  case 49:

    { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             ;}
    break;

  case 52:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 53:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             ;}
    break;

  case 57:

    { connectionDescription->type = (yyvsp[0]._connectionType); ;}
    break;

  case 58:

    { connectionDescription->hostname = (yyvsp[0]._string); ;}
    break;

  case 59:

    { connectionDescription->launchCommand = (yyvsp[0]._string); ;}
    break;

  case 60:

    { connectionDescription->launchTimeout = (yyvsp[0]._unsigned); ;}
    break;

  case 63:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 64:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 68:

    { eqPipe->setDisplay( (yyvsp[0]._unsigned) ); ;}
    break;

  case 69:

    { eqPipe->setScreen( (yyvsp[0]._unsigned) ); ;}
    break;

  case 70:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], (int)(yyvsp[0]._viewport)[1],
                                                      (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
        ;}
    break;

  case 73:

    { window = loader->createWindow(); ;}
    break;

  case 74:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 79:

    { window->setName( (yyvsp[0]._string) ); ;}
    break;

  case 80:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], 
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3])); 
        ;}
    break;

  case 89:

    { window->setIAttribute( eq::Window::IATTR_HINTS_STEREO, (yyvsp[0]._int) ); ;}
    break;

  case 90:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DOUBLEBUFFER, (yyvsp[0]._int) ); ;}
    break;

  case 91:

    { window->setIAttribute( eq::Window::IATTR_HINTS_FULLSCREEN, (yyvsp[0]._int) ); ;}
    break;

  case 92:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DECORATION, (yyvsp[0]._int) ); ;}
    break;

  case 96:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, (yyvsp[0]._int) ); ;}
    break;

  case 97:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, (yyvsp[0]._int) ); ;}
    break;

  case 98:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, (yyvsp[0]._int) ); ;}
    break;

  case 99:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, (yyvsp[0]._int) ); ;}
    break;

  case 102:

    { channel = loader->createChannel(); ;}
    break;

  case 103:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 107:

    { channel->setName( (yyvsp[0]._string) ); ;}
    break;

  case 108:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0],
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3]));
        ;}
    break;

  case 111:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;}
    break;

  case 112:

    { eqCompound = eqCompound->getParent(); ;}
    break;

  case 118:

    { eqCompound->setName( (yyvsp[0]._string) ); ;}
    break;

  case 119:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[0]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;}
    break;

  case 120:

    { eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 122:

    { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED ); ;}
    break;

  case 124:

    { flags = eq::Frame::BUFFER_UNDEFINED;;}
    break;

  case 125:

    { eqCompound->setBuffers( flags ); flags = 0; ;}
    break;

  case 126:

    { eqCompound->setViewport( eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3] )); ;}
    break;

  case 127:

    { eqCompound->setRange( eq::Range( (yyvsp[-2]._float), (yyvsp[-1]._float) )); ;}
    break;

  case 135:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 136:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 137:

    { eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);;}
    break;

  case 138:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK);;}
    break;

  case 142:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 143:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 144:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 148:

    { flags |= eq::Frame::BUFFER_COLOR; ;}
    break;

  case 149:

    { flags |= eq::Frame::BUFFER_DEPTH; ;}
    break;

  case 150:

    { 
        eq::Wall wall;
        wall.bottomLeft[0] = (yyvsp[-16]._float);
        wall.bottomLeft[1] = (yyvsp[-15]._float);
        wall.bottomLeft[2] = (yyvsp[-14]._float);

        wall.bottomRight[0] = (yyvsp[-10]._float);
        wall.bottomRight[1] = (yyvsp[-9]._float);
        wall.bottomRight[2] = (yyvsp[-8]._float);

        wall.topLeft[0] = (yyvsp[-4]._float);
        wall.topLeft[1] = (yyvsp[-3]._float);
        wall.topLeft[2] = (yyvsp[-2]._float);
        eqCompound->setWall( wall );
    ;}
    break;

  case 151:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 152:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 156:

    { swapBarrier->setName( (yyvsp[0]._string) ); ;}
    break;

  case 157:

    { frame = new eqs::Frame(); ;}
    break;

  case 158:

    { 
            eqCompound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 159:

    { frame = new eqs::Frame(); ;}
    break;

  case 160:

    { 
            eqCompound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 164:

    { frame->setName( (yyvsp[0]._string) ); ;}
    break;

  case 165:

    { frame->setViewport(eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3])); ;}
    break;

  case 166:

    { flags = eq::Frame::BUFFER_UNDEFINED; ;}
    break;

  case 167:

    { frame->setBuffers( flags ); flags = 0; ;}
    break;

  case 168:

    { 
         (yyval._viewport)[0] = (yyvsp[-4]._float);
         (yyval._viewport)[1] = (yyvsp[-3]._float);
         (yyval._viewport)[2] = (yyvsp[-2]._float);
         (yyval._viewport)[3] = (yyvsp[-1]._float);
     ;}
    break;

  case 169:

    { (yyval._int) = eq::ON; ;}
    break;

  case 170:

    { (yyval._int) = eq::OFF; ;}
    break;

  case 171:

    { (yyval._int) = eq::AUTO; ;}
    break;

  case 172:

    { (yyval._int) = (yyvsp[0]._int); ;}
    break;

  case 173:

    {
         stringBuf = yytext;
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 174:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 175:

    { (yyval._float) = (yyvsp[0]._int); ;}
    break;

  case 176:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 177:

    { (yyval._int) = (yyvsp[0]._unsigned); ;}
    break;

  case 178:

    { (yyval._unsigned) = atoi( yytext ); ;}
    break;


    }

/* Line 1037 of yacc.c.  */


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
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
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
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
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

