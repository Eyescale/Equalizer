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
     EQTOKEN_CONFIG_FATTR_EYE_BASE = 265,
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
     EQTOKEN_FORMAT = 305,
     EQTOKEN_CLEAR = 306,
     EQTOKEN_DRAW = 307,
     EQTOKEN_READBACK = 308,
     EQTOKEN_CYCLOP = 309,
     EQTOKEN_LEFT = 310,
     EQTOKEN_RIGHT = 311,
     EQTOKEN_VIEWPORT = 312,
     EQTOKEN_RANGE = 313,
     EQTOKEN_DISPLAY = 314,
     EQTOKEN_SCREEN = 315,
     EQTOKEN_WALL = 316,
     EQTOKEN_BOTTOM_LEFT = 317,
     EQTOKEN_BOTTOM_RIGHT = 318,
     EQTOKEN_TOP_LEFT = 319,
     EQTOKEN_SYNC = 320,
     EQTOKEN_LATENCY = 321,
     EQTOKEN_SWAPBARRIER = 322,
     EQTOKEN_OUTPUTFRAME = 323,
     EQTOKEN_INPUTFRAME = 324,
     EQTOKEN_STRING = 325,
     EQTOKEN_FLOAT = 326,
     EQTOKEN_INTEGER = 327,
     EQTOKEN_UNSIGNED = 328
   };
#endif
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_TYPE 259
#define EQTOKEN_CONNECTION_HOSTNAME 260
#define EQTOKEN_CONNECTION_TCPIP_PORT 261
#define EQTOKEN_CONNECTION_LAUNCH_TIMEOUT 262
#define EQTOKEN_CONNECTION_LAUNCH_COMMAND 263
#define EQTOKEN_WINDOW_IATTR_HINTS_STEREO 264
#define EQTOKEN_CONFIG_FATTR_EYE_BASE 265
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
#define EQTOKEN_FORMAT 305
#define EQTOKEN_CLEAR 306
#define EQTOKEN_DRAW 307
#define EQTOKEN_READBACK 308
#define EQTOKEN_CYCLOP 309
#define EQTOKEN_LEFT 310
#define EQTOKEN_RIGHT 311
#define EQTOKEN_VIEWPORT 312
#define EQTOKEN_RANGE 313
#define EQTOKEN_DISPLAY 314
#define EQTOKEN_SCREEN 315
#define EQTOKEN_WALL 316
#define EQTOKEN_BOTTOM_LEFT 317
#define EQTOKEN_BOTTOM_RIGHT 318
#define EQTOKEN_TOP_LEFT 319
#define EQTOKEN_SYNC 320
#define EQTOKEN_LATENCY 321
#define EQTOKEN_SWAPBARRIER 322
#define EQTOKEN_OUTPUTFRAME 323
#define EQTOKEN_INPUTFRAME 324
#define EQTOKEN_STRING 325
#define EQTOKEN_FLOAT 326
#define EQTOKEN_INTEGER 327
#define EQTOKEN_UNSIGNED 328




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
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   322

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  80
/* YYNRULES -- Number of rules. */
#define YYNRULES  174
/* YYNRULES -- Number of states. */
#define YYNSTATES  296

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   328

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
       2,    76,     2,    77,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    74,     2,    75,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
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
     367,   369,   371,   372,   374,   377,   379,   381,   383,   384,
     386,   389,   391,   393,   395,   396,   398,   401,   403,   405,
     427,   428,   434,   435,   437,   440,   443,   444,   450,   451,
     457,   458,   460,   463,   466,   473,   475,   477,   479,   481,
     483,   485,   487,   489,   491
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      79,     0,    -1,    80,    83,    -1,     3,    74,    81,    75,
      -1,    -1,    80,    -1,    81,    80,    -1,     4,    82,    -1,
       5,   154,    -1,     6,   157,    -1,     7,   157,    -1,     8,
     154,    -1,     9,   153,    -1,    11,   153,    -1,    12,   153,
      -1,    13,   153,    -1,    14,   153,    -1,    15,   153,    -1,
      16,   153,    -1,    17,   153,    -1,    10,   155,    -1,    43,
      -1,    -1,    18,    74,    84,    85,    75,    -1,    86,    -1,
      85,    86,    -1,    -1,    19,    74,    87,    88,    92,   126,
      75,    -1,    -1,    89,    -1,    88,    89,    -1,    66,   157,
      -1,    24,    74,    90,    75,    -1,    -1,    91,    -1,    90,
      91,    -1,    49,   155,    -1,    93,    -1,    92,    93,    -1,
      96,    -1,    94,    -1,    -1,    21,    74,    95,   100,    98,
     105,    75,    -1,    -1,    20,    74,    97,   100,    98,   105,
      75,    -1,    -1,    99,    -1,    98,    99,    -1,    -1,    -1,
     101,    -1,   100,   101,    -1,    -1,    40,    74,   102,   103,
      75,    -1,    -1,   104,    -1,   103,   104,    -1,    42,    82,
      -1,    44,   154,    -1,    45,   154,    -1,    46,   157,    -1,
     106,    -1,   105,   106,    -1,    -1,    22,    74,   107,   108,
     110,    75,    -1,    -1,   109,    -1,   108,   109,    -1,    59,
     157,    -1,    60,   157,    -1,    57,   152,    -1,   111,    -1,
     110,   111,    -1,    -1,    23,    74,   112,   113,   121,    75,
      -1,    -1,   114,    -1,   113,   114,    -1,    24,    74,   115,
      75,    -1,    41,   154,    -1,    57,   152,    -1,    -1,   116,
      -1,   115,   116,    -1,    25,    74,   117,    75,    -1,    30,
      74,   119,    75,    -1,    -1,   118,    -1,   117,   118,    -1,
      26,   153,    -1,    27,   153,    -1,    28,   153,    -1,    29,
     153,    -1,    -1,   120,    -1,   119,   120,    -1,    31,   153,
      -1,    32,   153,    -1,    33,   153,    -1,    34,   153,    -1,
     122,    -1,   121,   122,    -1,    -1,    38,    74,   123,   124,
      75,    -1,    -1,   125,    -1,   124,   125,    -1,    41,   154,
      -1,    57,   152,    -1,   127,    -1,   126,   127,    -1,    -1,
      39,    74,   128,   130,   129,   130,    75,    -1,    -1,   126,
      -1,    -1,   131,    -1,   130,   131,    -1,    41,   154,    -1,
      38,   154,    -1,    -1,    47,    76,   132,   135,    77,    -1,
      -1,    48,    76,   133,   137,    77,    -1,    -1,    50,    76,
     134,   139,    77,    -1,    57,   152,    -1,    58,    76,   155,
     155,    77,    -1,   141,    -1,   142,    -1,   146,    -1,   148,
      -1,    -1,   136,    -1,   135,   136,    -1,    51,    -1,    52,
      -1,    53,    -1,    -1,   138,    -1,   137,   138,    -1,    54,
      -1,    55,    -1,    56,    -1,    -1,   140,    -1,   139,   140,
      -1,    31,    -1,    33,    -1,    61,    74,    62,    76,   155,
     155,   155,    77,    63,    76,   155,   155,   155,    77,    64,
      76,   155,   155,   155,    77,    75,    -1,    -1,    67,    74,
     143,   144,    75,    -1,    -1,   145,    -1,   144,   145,    -1,
      41,   154,    -1,    -1,    68,    74,   147,   150,    75,    -1,
      -1,    69,    74,   149,   150,    75,    -1,    -1,   151,    -1,
     150,   151,    -1,    41,   154,    -1,    76,   155,   155,   155,
     155,    77,    -1,    35,    -1,    36,    -1,    37,    -1,   156,
      -1,    70,    -1,    71,    -1,   156,    -1,    72,    -1,   157,
      -1,    73,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
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
     391,   392,   394,   394,   394,   396,   397,   398,   400,   400,
     400,   402,   403,   404,   406,   406,   406,   408,   409,   411,
     432,   432,   438,   438,   439,   441,   443,   443,   449,   449,
     455,   455,   455,   457,   459,   468,   469,   470,   471,   473,
     478,   479,   481,   482,   483
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
  "EQTOKEN_WINDOW_IATTR_HINTS_STEREO", "EQTOKEN_CONFIG_FATTR_EYE_BASE", 
  "EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER", 
  "EQTOKEN_WINDOW_IATTR_HINTS_FULLSCREEN", 
  "EQTOKEN_WINDOW_IATTR_HINTS_DECORATION", 
  "EQTOKEN_WINDOW_IATTR_PLANES_COLOR", 
  "EQTOKEN_WINDOW_IATTR_PLANES_ALPHA", 
  "EQTOKEN_WINDOW_IATTR_PLANES_DEPTH", 
  "EQTOKEN_WINDOW_IATTR_PLANES_STENCIL", "EQTOKEN_SERVER", 
  "EQTOKEN_CONFIG", "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE", 
  "EQTOKEN_WINDOW", "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINTS", 
  "EQTOKEN_STEREO", "EQTOKEN_DOUBLEBUFFER", "EQTOKEN_FULLSCREEN", 
  "EQTOKEN_DECORATION", "EQTOKEN_PLANES", "EQTOKEN_COLOR", 
  "EQTOKEN_ALPHA", "EQTOKEN_DEPTH", "EQTOKEN_STENCIL", "EQTOKEN_ON", 
  "EQTOKEN_OFF", "EQTOKEN_AUTO", "EQTOKEN_CHANNEL", "EQTOKEN_COMPOUND", 
  "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE", "EQTOKEN_TCPIP", 
  "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND", "EQTOKEN_TIMEOUT", 
  "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_EYE_BASE", "EQTOKEN_FORMAT", 
  "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_READBACK", "EQTOKEN_CYCLOP", 
  "EQTOKEN_LEFT", "EQTOKEN_RIGHT", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE", 
  "EQTOKEN_DISPLAY", "EQTOKEN_SCREEN", "EQTOKEN_WALL", 
  "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT", "EQTOKEN_TOP_LEFT", 
  "EQTOKEN_SYNC", "EQTOKEN_LATENCY", "EQTOKEN_SWAPBARRIER", 
  "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME", "EQTOKEN_STRING", 
  "EQTOKEN_FLOAT", "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED", "'{'", "'}'", 
  "'['", "']'", "$accept", "file", "global", "globals", "connectionType", 
  "server", "@1", "configs", "config", "@2", "configFields", 
  "configField", "configAttributes", "configAttribute", "nodes", "node", 
  "otherNode", "@3", "appNode", "@4", "nodeFields", "nodeField", 
  "connections", "connection", "@5", "connectionFields", 
  "connectionField", "pipes", "pipe", "@6", "pipeFields", "pipeField", 
  "windows", "window", "@7", "windowFields", "windowField", 
  "windowAttributes", "windowAttribute", "windowHints", "windowHint", 
  "windowPlanes", "windowPlane", "channels", "channel", "@8", 
  "channelFields", "channelField", "compounds", "compound", "@9", 
  "compoundChildren", "compoundFields", "compoundField", "@10", "@11", 
  "@12", "compoundTasks", "compoundTask", "compoundEyes", "compoundEye", 
  "compoundFormats", "compoundFormat", "wall", "swapBarrier", "@13", 
  "swapBarrierFields", "swapBarrierField", "outputFrame", "@14", 
  "inputFrame", "@15", "frameFields", "frameField", "viewport", "IATTR", 
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   123,   125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    78,    79,    80,    80,    81,    81,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    82,    84,    83,    85,    85,    87,    86,    88,    88,
      88,    89,    89,    90,    90,    90,    91,    92,    92,    93,
      93,    95,    94,    97,    96,    98,    98,    98,    99,   100,
     100,   100,   102,   101,   103,   103,   103,   104,   104,   104,
     104,   105,   105,   107,   106,   108,   108,   108,   109,   109,
     109,   110,   110,   112,   111,   113,   113,   113,   114,   114,
     114,   115,   115,   115,   116,   116,   117,   117,   117,   118,
     118,   118,   118,   119,   119,   119,   120,   120,   120,   120,
     121,   121,   123,   122,   124,   124,   124,   125,   125,   126,
     126,   128,   127,   129,   129,   130,   130,   130,   131,   131,
     132,   131,   133,   131,   134,   131,   131,   131,   131,   131,
     131,   131,   135,   135,   135,   136,   136,   136,   137,   137,
     137,   138,   138,   138,   139,   139,   139,   140,   140,   141,
     143,   142,   144,   144,   144,   145,   147,   146,   149,   148,
     150,   150,   150,   151,   152,   153,   153,   153,   153,   154,
     155,   155,   156,   156,   157
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
       1,     1,     0,     1,     2,     1,     1,     1,     0,     1,
       2,     1,     1,     1,     0,     1,     2,     1,     1,    21,
       0,     5,     0,     1,     2,     2,     0,     5,     0,     5,
       0,     1,     2,     2,     6,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,    21,
       7,   169,     8,   174,     9,    10,    11,   165,   166,   167,
     172,    12,   168,   173,   170,    20,   171,    13,    14,    15,
      16,    17,    18,    19,     1,     0,     2,     5,     0,    22,
       3,     6,     0,     0,     0,    24,    26,    23,    25,    28,
       0,     0,     0,    29,    33,    31,     0,     0,    30,     0,
      37,    40,    39,     0,     0,    34,    43,    41,     0,    38,
       0,   109,    36,    32,    35,    49,    49,   111,    27,   110,
       0,    45,    50,    45,   115,    52,     0,    46,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   113,   116,   128,   129,   130,   131,    54,     0,    47,
       0,    61,     0,   119,   118,   120,   122,   124,     0,   126,
       0,     0,   150,   156,   158,   114,   115,   117,     0,     0,
       0,     0,     0,    55,    63,    44,    62,    42,   132,   138,
     144,     0,     0,     0,   152,   160,   160,     0,    57,    58,
      59,    60,    53,    56,    65,   135,   136,   137,     0,   133,
     141,   142,   143,     0,   139,   147,   148,     0,   145,     0,
       0,     0,     0,     0,   153,     0,     0,   161,     0,   112,
       0,     0,     0,     0,    66,   121,   134,   123,   140,   125,
     146,     0,   127,     0,   155,   151,   154,   163,   157,   162,
     159,    70,    68,    69,     0,    67,     0,    71,     0,     0,
      73,    64,    72,   164,     0,    75,     0,     0,     0,     0,
       0,    76,     0,    81,    79,    80,     0,    77,     0,   100,
       0,     0,     0,     0,    82,   102,    74,   101,     0,    86,
      93,    78,    83,   104,     0,     0,     0,     0,     0,     0,
      87,     0,     0,     0,     0,     0,    94,     0,     0,     0,
     105,     0,    89,    90,    91,    92,    84,    88,    96,    97,
      98,    99,    85,    95,   107,   108,   103,   106,     0,     0,
       0,     0,     0,     0,     0,   149
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    16,    17,    48,    20,    46,    52,    54,    55,    59,
      62,    63,    74,    75,    69,    70,    71,    86,    72,    85,
      96,    97,    91,    92,   117,   142,   143,   120,   121,   164,
     193,   194,   216,   217,   225,   230,   231,   243,   244,   259,
     260,   265,   266,   238,   239,   253,   269,   270,    80,    81,
      94,   136,   111,   112,   148,   149,   150,   168,   169,   173,
     174,   177,   178,   113,   114,   154,   183,   184,   115,   155,
     116,   156,   186,   187,   129,    31,    22,    35,    36,    33
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -190
static const short yypact[] =
{
     228,    11,    50,    33,    55,    55,    33,    36,    88,    36,
      36,    36,    36,    36,    36,    36,   106,   102,   228,  -190,
    -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,
    -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,
    -190,  -190,  -190,  -190,  -190,    68,  -190,  -190,    21,  -190,
    -190,  -190,   112,    69,   -16,  -190,  -190,  -190,  -190,    20,
      76,    55,    32,  -190,   114,  -190,   127,   128,  -190,    90,
    -190,  -190,  -190,    88,     9,  -190,  -190,  -190,   131,  -190,
       6,  -190,  -190,  -190,  -190,   172,   172,  -190,  -190,  -190,
     147,   172,  -190,   172,   234,  -190,   198,  -190,  -190,   198,
      33,    33,   148,   149,   150,   151,   170,   173,   174,   180,
     181,   146,  -190,  -190,  -190,  -190,  -190,   129,   186,  -190,
      -8,  -190,     1,  -190,  -190,  -190,  -190,  -190,    88,  -190,
      88,   161,  -190,  -190,  -190,   210,   234,  -190,    50,    33,
      33,    55,    81,  -190,  -190,  -190,  -190,  -190,   117,   155,
     175,    88,    88,   190,   226,   232,   232,    97,  -190,  -190,
    -190,  -190,  -190,  -190,   138,  -190,  -190,  -190,   100,  -190,
    -190,  -190,  -190,    60,  -190,  -190,  -190,    10,  -190,    88,
     197,    88,    33,    24,  -190,    33,    37,  -190,    47,  -190,
     151,    55,    55,    23,  -190,  -190,  -190,  -190,  -190,  -190,
    -190,    88,  -190,    88,  -190,  -190,  -190,  -190,  -190,  -190,
    -190,  -190,  -190,  -190,   202,  -190,    16,  -190,   200,    88,
    -190,  -190,  -190,  -190,   201,    89,   216,   206,    33,   151,
      83,  -190,   207,   137,  -190,  -190,   211,  -190,    26,  -190,
      88,   212,   213,    17,  -190,  -190,  -190,  -190,    88,   154,
     157,  -190,  -190,    91,    88,    36,    36,    36,    36,    22,
    -190,    36,    36,    36,    36,    29,  -190,    33,   151,    61,
    -190,   217,  -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,
    -190,  -190,  -190,  -190,  -190,  -190,  -190,  -190,   224,   214,
      88,    88,    88,   219,   218,  -190
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -190,  -190,    57,  -190,   159,  -190,  -190,  -190,   235,  -190,
    -190,   236,  -190,   225,  -190,   231,  -190,  -190,  -190,  -190,
     215,   -22,   220,   -36,  -190,  -190,   162,   208,    96,  -190,
    -190,   116,  -190,    94,  -190,  -190,    75,  -190,    70,  -190,
      52,  -190,    49,  -190,    74,  -190,  -190,    46,   205,   -65,
    -190,  -190,   182,   -57,  -190,  -190,  -190,  -190,   152,  -190,
     144,  -190,   142,  -190,  -190,  -190,  -190,   139,  -190,  -190,
    -190,  -190,   165,    31,  -189,     7,    -6,   -62,    -5,     8
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
      26,   211,    32,    53,    32,    32,    32,    32,    32,    32,
      32,    82,    24,    25,   118,    89,    37,    38,    39,    40,
      41,    42,    43,   118,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,   214,
     235,   175,   241,   176,    60,    78,   214,   242,   255,   256,
     257,   258,    66,    67,   137,    98,    60,    98,    73,    57,
     261,   262,   263,   264,   236,   182,   151,   145,   152,    65,
      89,    27,    28,    29,   119,    47,   147,   119,   185,   285,
     190,    88,   191,   192,    83,    18,    61,   199,   185,   179,
     180,   221,   251,    19,   123,   124,    50,   276,    61,   205,
     137,   246,   267,    21,   282,    51,    44,   227,    30,    23,
      66,    67,   208,   227,   170,   171,   172,   201,   268,   203,
      45,   236,   210,   138,   228,   139,   140,   141,    23,    78,
     228,    53,   267,   159,   160,   100,   286,   197,   101,   218,
     229,   219,    49,    56,   102,   103,   229,   104,   268,   161,
      64,   165,   166,   167,   105,   106,   162,   224,   107,    34,
      30,    23,   241,    73,   108,   109,   110,   242,   165,   166,
     167,   138,   189,   139,   140,   141,   204,   195,   248,   207,
     255,   256,   257,   258,   100,    78,   254,   101,   261,   262,
     263,   264,   271,   102,   103,   190,   104,   191,   192,   212,
     213,    76,    77,   105,   106,    87,   175,   107,   176,   170,
     171,   172,    90,   108,   109,   110,   146,   209,   146,   209,
     118,    95,   234,   153,   125,   126,   127,   128,   291,   292,
     293,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,   130,   131,   132,    78,
      32,    32,    32,    32,   133,   134,    32,    32,    32,    32,
     144,   284,   272,   273,   274,   275,   181,   182,   278,   279,
     280,   281,   100,   185,   202,   101,   220,   223,   226,   232,
     233,   102,   103,   240,   104,   245,   249,   250,   289,    58,
     290,   105,   106,   295,   288,   107,   294,   158,    68,    84,
      79,   108,   109,   110,   163,   237,    93,   122,    99,   215,
     222,   277,   247,   252,   283,   287,   135,   198,   157,   200,
     196,   188,   206
};

static const unsigned short yycheck[] =
{
       6,   190,     7,    19,     9,    10,    11,    12,    13,    14,
      15,    73,     4,     5,    22,    80,     9,    10,    11,    12,
      13,    14,    15,    22,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    23,
     229,    31,    25,    33,    24,    39,    23,    30,    26,    27,
      28,    29,    20,    21,   111,    91,    24,    93,    49,    75,
      31,    32,    33,    34,    38,    41,   128,    75,   130,    61,
     135,    35,    36,    37,    96,    18,    75,    99,    41,   268,
      57,    75,    59,    60,    75,    74,    66,    77,    41,   151,
     152,    75,    75,    43,   100,   101,    75,    75,    66,    75,
     157,    75,    41,    70,    75,    48,     0,    24,    72,    73,
      20,    21,    75,    24,    54,    55,    56,   179,    57,   181,
      18,    38,    75,    42,    41,    44,    45,    46,    73,    39,
      41,    19,    41,   139,   140,    38,    75,    77,    41,   201,
      57,   203,    74,    74,    47,    48,    57,    50,    57,   141,
      74,    51,    52,    53,    57,    58,    75,   219,    61,    71,
      72,    73,    25,    49,    67,    68,    69,    30,    51,    52,
      53,    42,    75,    44,    45,    46,   182,    77,   240,   185,
      26,    27,    28,    29,    38,    39,   248,    41,    31,    32,
      33,    34,   254,    47,    48,    57,    50,    59,    60,   191,
     192,    74,    74,    57,    58,    74,    31,    61,    33,    54,
      55,    56,    40,    67,    68,    69,   120,   186,   122,   188,
      22,    74,   228,    62,    76,    76,    76,    76,   290,   291,
     292,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    76,    74,    74,    39,
     255,   256,   257,   258,    74,    74,   261,   262,   263,   264,
      74,   267,   255,   256,   257,   258,    76,    41,   261,   262,
     263,   264,    38,    41,    77,    41,    74,    77,    77,    63,
      74,    47,    48,    76,    50,    74,    74,    74,    64,    54,
      76,    57,    58,    75,    77,    61,    77,   138,    62,    74,
      69,    67,    68,    69,   142,   230,    86,    99,    93,   193,
     216,   259,   238,   243,   265,   269,   111,   173,   136,   177,
     168,   156,   183
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    79,    80,    74,    43,
      82,    70,   154,    73,   157,   157,   154,    35,    36,    37,
      72,   153,   156,   157,    71,   155,   156,   153,   153,   153,
     153,   153,   153,   153,     0,    18,    83,    80,    81,    74,
      75,    80,    84,    19,    85,    86,    74,    75,    86,    87,
      24,    66,    88,    89,    74,   157,    20,    21,    89,    92,
      93,    94,    96,    49,    90,    91,    74,    74,    39,    93,
     126,   127,   155,    75,    91,    97,    95,    74,    75,   127,
      40,   100,   101,   100,   128,    74,    98,    99,   101,    98,
      38,    41,    47,    48,    50,    57,    58,    61,    67,    68,
      69,   130,   131,   141,   142,   146,   148,   102,    22,    99,
     105,   106,   105,   154,   154,    76,    76,    76,    76,   152,
      76,    74,    74,    74,    74,   126,   129,   131,    42,    44,
      45,    46,   103,   104,    74,    75,   106,    75,   132,   133,
     134,   155,   155,    62,   143,   147,   149,   130,    82,   154,
     154,   157,    75,   104,   107,    51,    52,    53,   135,   136,
      54,    55,    56,   137,   138,    31,    33,   139,   140,   155,
     155,    76,    41,   144,   145,    41,   150,   151,   150,    75,
      57,    59,    60,   108,   109,    77,   136,    77,   138,    77,
     140,   155,    77,   155,   154,    75,   145,   154,    75,   151,
      75,   152,   157,   157,    23,   109,   110,   111,   155,   155,
      74,    75,   111,    77,   155,   112,    77,    24,    41,    57,
     113,   114,    63,    74,   154,   152,    38,   114,   121,   122,
      76,    25,    30,   115,   116,    74,    75,   122,   155,    74,
      74,    75,   116,   123,   155,    26,    27,    28,    29,   117,
     118,    31,    32,    33,    34,   119,   120,    41,    57,   124,
     125,   155,   153,   153,   153,   153,    75,   118,   153,   153,
     153,   153,    75,   120,   154,   152,    75,   125,    77,    64,
      76,   155,   155,   155,    77,    75
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

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_STEREO, yyvsp[0]._int );
     ;}
    break;

  case 13:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DOUBLEBUFFER, yyvsp[0]._int );
     ;}
    break;

  case 14:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_FULLSCREEN, yyvsp[0]._int );
     ;}
    break;

  case 15:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DECORATION, yyvsp[0]._int );
     ;}
    break;

  case 16:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, yyvsp[0]._int );
     ;}
    break;

  case 17:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, yyvsp[0]._int );
     ;}
    break;

  case 18:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, yyvsp[0]._int );
     ;}
    break;

  case 19:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, yyvsp[0]._int );
     ;}
    break;

  case 20:

    {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, yyvsp[0]._float );
     ;}
    break;

  case 21:

    { yyval._connectionType = eqNet::Connection::TYPE_TCPIP; ;}
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

    { config->setLatency( yyvsp[0]._unsigned ); ;}
    break;

  case 36:

    { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, yyvsp[0]._float ); ;}
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

    { connectionDescription->type = yyvsp[0]._connectionType; ;}
    break;

  case 58:

    { connectionDescription->hostname = yyvsp[0]._string; ;}
    break;

  case 59:

    { connectionDescription->launchCommand = yyvsp[0]._string; ;}
    break;

  case 60:

    { connectionDescription->launchTimeout = yyvsp[0]._unsigned; ;}
    break;

  case 63:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 64:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 68:

    { eqPipe->setDisplay( yyvsp[0]._unsigned ); ;}
    break;

  case 69:

    { eqPipe->setScreen( yyvsp[0]._unsigned ); ;}
    break;

  case 70:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], (int)yyvsp[0]._viewport[1],
                                                      (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
        ;}
    break;

  case 73:

    { window = loader->createWindow(); ;}
    break;

  case 74:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 79:

    { window->setName( yyvsp[0]._string ); ;}
    break;

  case 80:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0], 
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                window->setViewport( eq::Viewport(yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); 
        ;}
    break;

  case 89:

    { window->setIAttribute( eq::Window::IATTR_HINTS_STEREO, yyvsp[0]._int ); ;}
    break;

  case 90:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DOUBLEBUFFER, yyvsp[0]._int ); ;}
    break;

  case 91:

    { window->setIAttribute( eq::Window::IATTR_HINTS_FULLSCREEN, yyvsp[0]._int ); ;}
    break;

  case 92:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DECORATION, yyvsp[0]._int ); ;}
    break;

  case 96:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, yyvsp[0]._int ); ;}
    break;

  case 97:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, yyvsp[0]._int ); ;}
    break;

  case 98:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, yyvsp[0]._int ); ;}
    break;

  case 99:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, yyvsp[0]._int ); ;}
    break;

  case 102:

    { channel = loader->createChannel(); ;}
    break;

  case 103:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 107:

    { channel->setName( yyvsp[0]._string ); ;}
    break;

  case 108:

    {
            if( yyvsp[0]._viewport[2] > 1 || yyvsp[0]._viewport[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)yyvsp[0]._viewport[0],
                                          (int)yyvsp[0]._viewport[1], (int)yyvsp[0]._viewport[2], (int)yyvsp[0]._viewport[3] ));
            else
                channel->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3]));
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

    { eqCompound->setName( yyvsp[0]._string ); ;}
    break;

  case 119:

    {
         eqs::Channel* channel = config->findChannel( yyvsp[0]._string );
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

    { eqCompound->setFormats( eq::Frame::FORMAT_UNDEFINED );;}
    break;

  case 126:

    { eqCompound->setViewport( eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3] )); ;}
    break;

  case 127:

    { eqCompound->setRange( eq::Range( yyvsp[-2]._float, yyvsp[-1]._float )); ;}
    break;

  case 135:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 136:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 137:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK ); ;}
    break;

  case 141:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 142:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 143:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 147:

    { eqCompound->enableFormat( eq::Frame::FORMAT_COLOR ); ;}
    break;

  case 148:

    { eqCompound->enableFormat( eq::Frame::FORMAT_DEPTH ); ;}
    break;

  case 149:

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
    ;}
    break;

  case 150:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 151:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 155:

    { swapBarrier->setName( yyvsp[0]._string ); ;}
    break;

  case 156:

    { frame = new eqs::Frame(); ;}
    break;

  case 157:

    { 
            eqCompound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 158:

    { frame = new eqs::Frame(); ;}
    break;

  case 159:

    { 
            eqCompound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 163:

    { frame->setName( yyvsp[0]._string ); ;}
    break;

  case 164:

    { 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;}
    break;

  case 165:

    { yyval._int = eq::ON; ;}
    break;

  case 166:

    { yyval._int = eq::OFF; ;}
    break;

  case 167:

    { yyval._int = eq::AUTO; ;}
    break;

  case 168:

    { yyval._int = yyvsp[0]._int; ;}
    break;

  case 169:

    {
         stringBuf = yytext;
         yyval._string = stringBuf.c_str(); 
     ;}
    break;

  case 170:

    { yyval._float = atof( yytext ); ;}
    break;

  case 171:

    { yyval._float = yyvsp[0]._int; ;}
    break;

  case 172:

    { yyval._int = atoi( yytext ); ;}
    break;

  case 173:

    { yyval._int = yyvsp[0]._unsigned; ;}
    break;

  case 174:

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

