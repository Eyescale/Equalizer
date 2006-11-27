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
#define YYLAST   336

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  81
/* YYNRULES -- Number of rules. */
#define YYNRULES  177
/* YYNRULES -- Number of states. */
#define YYNSTATES  303

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
     457,   458,   460,   463,   466,   469,   470,   476,   483,   485,
     487,   489,   491,   493,   495,   497,   499,   501
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      79,     0,    -1,    80,    83,    -1,     3,    74,    81,    75,
      -1,    -1,    80,    -1,    81,    80,    -1,     4,   155,    -1,
       5,   155,    -1,     6,    82,    -1,     7,   158,    -1,     8,
     158,    -1,    10,   154,    -1,    11,   154,    -1,    12,   154,
      -1,    13,   154,    -1,    14,   154,    -1,    15,   154,    -1,
      16,   154,    -1,    17,   154,    -1,     9,   156,    -1,    43,
      -1,    -1,    18,    74,    84,    85,    75,    -1,    86,    -1,
      85,    86,    -1,    -1,    19,    74,    87,    88,    92,   126,
      75,    -1,    -1,    89,    -1,    88,    89,    -1,    66,   158,
      -1,    24,    74,    90,    75,    -1,    -1,    91,    -1,    90,
      91,    -1,    49,   156,    -1,    93,    -1,    92,    93,    -1,
      96,    -1,    94,    -1,    -1,    21,    74,    95,   100,    98,
     105,    75,    -1,    -1,    20,    74,    97,   100,    98,   105,
      75,    -1,    -1,    99,    -1,    98,    99,    -1,    -1,    -1,
     101,    -1,   100,   101,    -1,    -1,    40,    74,   102,   103,
      75,    -1,    -1,   104,    -1,   103,   104,    -1,    42,    82,
      -1,    44,   155,    -1,    45,   155,    -1,    46,   158,    -1,
     106,    -1,   105,   106,    -1,    -1,    22,    74,   107,   108,
     110,    75,    -1,    -1,   109,    -1,   108,   109,    -1,    59,
     158,    -1,    60,   158,    -1,    57,   153,    -1,   111,    -1,
     110,   111,    -1,    -1,    23,    74,   112,   113,   121,    75,
      -1,    -1,   114,    -1,   113,   114,    -1,    24,    74,   115,
      75,    -1,    41,   155,    -1,    57,   153,    -1,    -1,   116,
      -1,   115,   116,    -1,    25,    74,   117,    75,    -1,    30,
      74,   119,    75,    -1,    -1,   118,    -1,   117,   118,    -1,
      26,   154,    -1,    27,   154,    -1,    28,   154,    -1,    29,
     154,    -1,    -1,   120,    -1,   119,   120,    -1,    31,   154,
      -1,    32,   154,    -1,    33,   154,    -1,    34,   154,    -1,
     122,    -1,   121,   122,    -1,    -1,    38,    74,   123,   124,
      75,    -1,    -1,   125,    -1,   124,   125,    -1,    41,   155,
      -1,    57,   153,    -1,   127,    -1,   126,   127,    -1,    -1,
      39,    74,   128,   130,   129,   130,    75,    -1,    -1,   126,
      -1,    -1,   131,    -1,   130,   131,    -1,    41,   155,    -1,
      38,   155,    -1,    -1,    47,    76,   132,   135,    77,    -1,
      -1,    48,    76,   133,   137,    77,    -1,    -1,    50,    76,
     134,   139,    77,    -1,    57,   153,    -1,    58,    76,   156,
     156,    77,    -1,   141,    -1,   142,    -1,   146,    -1,   148,
      -1,    -1,   136,    -1,   135,   136,    -1,    51,    -1,    52,
      -1,    53,    -1,    -1,   138,    -1,   137,   138,    -1,    54,
      -1,    55,    -1,    56,    -1,    -1,   140,    -1,   139,   140,
      -1,    31,    -1,    33,    -1,    61,    74,    62,    76,   156,
     156,   156,    77,    63,    76,   156,   156,   156,    77,    64,
      76,   156,   156,   156,    77,    75,    -1,    -1,    67,    74,
     143,   144,    75,    -1,    -1,   145,    -1,   144,   145,    -1,
      41,   155,    -1,    -1,    68,    74,   147,   150,    75,    -1,
      -1,    69,    74,   149,   150,    75,    -1,    -1,   151,    -1,
     150,   151,    -1,    41,   155,    -1,    57,   153,    -1,    -1,
      50,    76,   152,   139,    77,    -1,    76,   156,   156,   156,
     156,    77,    -1,    35,    -1,    36,    -1,    37,    -1,   157,
      -1,    70,    -1,    71,    -1,   157,    -1,    72,    -1,   158,
      -1,    73,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   142,   142,   144,   145,   148,   148,   151,   156,   161,
     166,   171,   176,   181,   186,   191,   196,   201,   206,   211,
     216,   222,   224,   224,   227,   227,   228,   228,   231,   231,
     231,   233,   234,   235,   235,   235,   237,   240,   240,   241,
     241,   242,   242,   246,   246,   250,   250,   250,   251,   254,
     258,   258,   260,   259,   266,   266,   267,   269,   270,   271,
     272,   275,   275,   276,   276,   279,   279,   279,   281,   282,
     283,   289,   289,   290,   290,   293,   293,   293,   295,   297,
     298,   306,   306,   306,   308,   309,   310,   310,   310,   312,
     314,   316,   318,   320,   320,   320,   322,   324,   326,   328,
     331,   331,   332,   332,   335,   336,   336,   338,   339,   349,
     349,   351,   350,   364,   364,   366,   366,   367,   369,   370,
     378,   378,   380,   380,   382,   382,   384,   386,   388,   389,
     390,   391,   393,   393,   393,   395,   396,   397,   399,   399,
     399,   401,   402,   403,   405,   405,   405,   407,   408,   410,
     431,   431,   437,   437,   438,   440,   442,   442,   448,   448,
     454,   454,   454,   456,   457,   459,   459,   462,   471,   472,
     473,   474,   476,   481,   482,   484,   485,   486
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
  "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_EYE_BASE", "EQTOKEN_BUFFER", 
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
  "buffers", "buffer", "wall", "swapBarrier", "@13", "swapBarrierFields", 
  "swapBarrierField", "outputFrame", "@14", "inputFrame", "@15", 
  "frameFields", "frameField", "@16", "viewport", "IATTR", "STRING", 
  "FLOAT", "INTEGER", "UNSIGNED", 0
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
     150,   150,   150,   151,   151,   152,   151,   153,   154,   154,
     154,   154,   155,   156,   156,   157,   157,   158
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
       0,     1,     2,     2,     2,     0,     5,     6,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     4,   172,
       7,     8,    21,     9,   177,    10,    11,   173,   175,    20,
     174,   176,   168,   169,   170,    12,   171,    13,    14,    15,
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
       0,     0,     0,     0,   153,     0,     0,     0,     0,   161,
       0,   112,     0,     0,     0,     0,    66,   121,   134,   123,
     140,   125,   146,     0,   127,     0,   155,   151,   154,   163,
     165,   164,   157,   162,   159,    70,    68,    69,     0,    67,
       0,    71,     0,     0,   144,    73,    64,    72,   167,     0,
       0,    75,     0,   166,     0,     0,     0,     0,    76,     0,
      81,    79,    80,     0,    77,     0,   100,     0,     0,     0,
       0,    82,   102,    74,   101,     0,    86,    93,    78,    83,
     104,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,     0,     0,    94,     0,     0,     0,   105,     0,    89,
      90,    91,    92,    84,    88,    96,    97,    98,    99,    85,
      95,   107,   108,   103,   106,     0,     0,     0,     0,     0,
       0,     0,   149
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    16,    17,    48,    23,    46,    52,    54,    55,    59,
      62,    63,    74,    75,    69,    70,    71,    86,    72,    85,
      96,    97,    91,    92,   117,   142,   143,   120,   121,   164,
     195,   196,   220,   221,   231,   237,   238,   250,   251,   266,
     267,   272,   273,   245,   246,   260,   276,   277,    80,    81,
      94,   136,   111,   112,   148,   149,   150,   168,   169,   173,
     174,   177,   178,   113,   114,   154,   183,   184,   115,   155,
     116,   156,   188,   189,   224,   129,    35,    20,    29,    30,
      31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -168
static const short yypact[] =
{
     237,   -29,   -21,   -21,    14,    22,    22,   213,    37,    37,
      37,    37,    37,    37,    37,    37,   106,    89,   237,  -168,
    -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,
    -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,
    -168,  -168,  -168,  -168,  -168,    81,  -168,  -168,    24,  -168,
    -168,  -168,    93,    84,     2,  -168,  -168,  -168,  -168,    20,
     112,    22,    26,  -168,   140,  -168,   116,   124,  -168,   129,
    -168,  -168,  -168,   213,    82,  -168,  -168,  -168,   127,  -168,
      16,  -168,  -168,  -168,  -168,   121,   121,  -168,  -168,  -168,
     128,   121,  -168,   121,   170,  -168,   158,  -168,  -168,   158,
     -21,   -21,   130,   133,   143,   149,   150,   155,   156,   162,
     185,   146,  -168,  -168,  -168,  -168,  -168,   132,   186,  -168,
       0,  -168,     1,  -168,  -168,  -168,  -168,  -168,   213,  -168,
     213,   154,  -168,  -168,  -168,   166,   170,  -168,    14,   -21,
     -21,    22,   120,  -168,  -168,  -168,  -168,  -168,   236,   238,
     142,   213,   213,   189,   229,   131,   131,    85,  -168,  -168,
    -168,  -168,  -168,  -168,   223,  -168,  -168,  -168,    50,  -168,
    -168,  -168,  -168,   115,  -168,  -168,  -168,    27,  -168,   213,
     204,   213,   -21,    15,  -168,   -21,   214,   149,    21,  -168,
      88,  -168,   149,    22,    22,    61,  -168,  -168,  -168,  -168,
    -168,  -168,  -168,   213,  -168,   213,  -168,  -168,  -168,  -168,
    -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,   217,  -168,
      19,  -168,   222,   213,   142,  -168,  -168,  -168,  -168,   224,
      28,   110,   239,  -168,   226,   -21,   149,    87,  -168,   227,
      38,  -168,  -168,   230,  -168,    47,  -168,   213,   231,   232,
      18,  -168,  -168,  -168,  -168,   213,   195,   245,  -168,  -168,
      99,   213,    37,    37,    37,    37,    25,  -168,    37,    37,
      37,    37,    49,  -168,   -21,   149,    73,  -168,   233,  -168,
    -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,  -168,
    -168,  -168,  -168,  -168,  -168,   243,   235,   213,   213,   213,
     240,   234,  -168
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -168,  -168,    71,  -168,   174,  -168,  -168,  -168,   254,  -168,
    -168,   251,  -168,   241,  -168,   247,  -168,  -168,  -168,  -168,
     221,    17,   242,   119,  -168,  -168,   176,   220,   175,  -168,
    -168,   125,  -168,   101,  -168,  -168,    86,  -168,    72,  -168,
      58,  -168,    53,  -168,    90,  -168,  -168,    51,   215,   -70,
    -168,  -168,   193,   -87,  -168,  -168,  -168,  -168,   163,  -168,
     157,   109,  -151,  -168,  -168,  -168,  -168,   151,  -168,  -168,
    -168,  -168,   180,   108,  -168,  -167,     4,    -3,   -64,    -7,
       6
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
      21,    36,    36,    36,    36,    36,    36,    36,    36,    82,
      89,    25,    26,    37,    38,    39,    40,    41,    42,    43,
     211,    53,   118,   118,   137,   215,   202,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,   218,   248,    60,    18,    66,    67,   249,    19,
      60,   262,   263,   264,   265,    78,   182,    22,   175,   175,
     176,   176,   185,   248,   151,    89,   152,    65,   249,   242,
     137,   186,    32,    33,    34,   145,   147,    57,   187,   202,
     268,   269,   270,   271,   218,   243,    61,   179,   180,    47,
     207,    88,    61,   258,   226,    24,   212,   123,   124,    50,
     283,   165,   166,   167,   201,   233,    44,    45,   292,    28,
      24,   234,    53,   119,   274,   203,   119,   205,   192,    51,
     193,   194,   253,   100,   289,   243,   101,   197,   235,   185,
     275,    73,   102,   103,   234,   104,   159,   160,   186,   222,
     274,   223,   105,   106,   236,   187,   107,   161,   293,    66,
      67,   235,   108,   109,   110,    49,   275,    83,    56,   229,
     191,    90,   138,   214,   139,   140,   141,   236,    78,   170,
     171,   172,   185,   175,   138,   176,   139,   140,   141,   206,
     118,   186,   209,   255,   100,    78,    64,   101,   187,    73,
      76,   261,   199,   102,   103,   162,   104,   278,    77,   216,
     217,    87,    95,   105,   106,    78,   125,   107,   100,   126,
      98,   101,    98,   108,   109,   110,   153,   102,   103,   127,
     104,   262,   263,   264,   265,   128,   130,   105,   106,   131,
     132,   107,   241,   298,   299,   300,   133,   108,   109,   110,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    36,    36,    36,    36,   134,
     144,    36,    36,    36,    36,   181,   279,   280,   281,   282,
     182,   291,   285,   286,   287,   288,   268,   269,   270,   271,
     192,   204,   193,   194,    27,    28,    24,   165,   166,   167,
     210,   225,   170,   171,   172,   146,   213,   146,   213,   228,
     240,   232,   239,   247,   252,   256,   257,   296,    58,   302,
     295,   297,   158,    68,    99,    84,    79,   301,   163,   122,
     219,   227,   259,   244,   284,   290,   135,   294,    93,   157,
     200,   198,     0,   230,   208,   254,   190
};

static const short yycheck[] =
{
       3,     8,     9,    10,    11,    12,    13,    14,    15,    73,
      80,     5,     6,     9,    10,    11,    12,    13,    14,    15,
     187,    19,    22,    22,   111,   192,   177,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    23,    25,    24,    74,    20,    21,    30,    70,
      24,    26,    27,    28,    29,    39,    41,    43,    31,    31,
      33,    33,    41,    25,   128,   135,   130,    61,    30,   236,
     157,    50,    35,    36,    37,    75,    75,    75,    57,   230,
      31,    32,    33,    34,    23,    38,    66,   151,   152,    18,
      75,    75,    66,    75,    75,    73,    75,   100,   101,    75,
      75,    51,    52,    53,    77,    77,     0,    18,   275,    72,
      73,    24,    19,    96,    41,   179,    99,   181,    57,    48,
      59,    60,    75,    38,    75,    38,    41,    77,    41,    41,
      57,    49,    47,    48,    24,    50,   139,   140,    50,   203,
      41,   205,    57,    58,    57,    57,    61,   141,    75,    20,
      21,    41,    67,    68,    69,    74,    57,    75,    74,   223,
      75,    40,    42,    75,    44,    45,    46,    57,    39,    54,
      55,    56,    41,    31,    42,    33,    44,    45,    46,   182,
      22,    50,   185,   247,    38,    39,    74,    41,    57,    49,
      74,   255,    77,    47,    48,    75,    50,   261,    74,   193,
     194,    74,    74,    57,    58,    39,    76,    61,    38,    76,
      91,    41,    93,    67,    68,    69,    62,    47,    48,    76,
      50,    26,    27,    28,    29,    76,    76,    57,    58,    74,
      74,    61,   235,   297,   298,   299,    74,    67,    68,    69,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,   262,   263,   264,   265,    74,
      74,   268,   269,   270,   271,    76,   262,   263,   264,   265,
      41,   274,   268,   269,   270,   271,    31,    32,    33,    34,
      57,    77,    59,    60,    71,    72,    73,    51,    52,    53,
      76,    74,    54,    55,    56,   120,   188,   122,   190,    77,
      74,    77,    63,    76,    74,    74,    74,    64,    54,    75,
      77,    76,   138,    62,    93,    74,    69,    77,   142,    99,
     195,   220,   250,   237,   266,   272,   111,   276,    86,   136,
     173,   168,    -1,   224,   183,   245,   156
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    79,    80,    74,    70,
     155,   155,    43,    82,    73,   158,   158,    71,    72,   156,
     157,   158,    35,    36,    37,   154,   157,   154,   154,   154,
     154,   154,   154,   154,     0,    18,    83,    80,    81,    74,
      75,    80,    84,    19,    85,    86,    74,    75,    86,    87,
      24,    66,    88,    89,    74,   158,    20,    21,    89,    92,
      93,    94,    96,    49,    90,    91,    74,    74,    39,    93,
     126,   127,   156,    75,    91,    97,    95,    74,    75,   127,
      40,   100,   101,   100,   128,    74,    98,    99,   101,    98,
      38,    41,    47,    48,    50,    57,    58,    61,    67,    68,
      69,   130,   131,   141,   142,   146,   148,   102,    22,    99,
     105,   106,   105,   155,   155,    76,    76,    76,    76,   153,
      76,    74,    74,    74,    74,   126,   129,   131,    42,    44,
      45,    46,   103,   104,    74,    75,   106,    75,   132,   133,
     134,   156,   156,    62,   143,   147,   149,   130,    82,   155,
     155,   158,    75,   104,   107,    51,    52,    53,   135,   136,
      54,    55,    56,   137,   138,    31,    33,   139,   140,   156,
     156,    76,    41,   144,   145,    41,    50,    57,   150,   151,
     150,    75,    57,    59,    60,   108,   109,    77,   136,    77,
     138,    77,   140,   156,    77,   156,   155,    75,   145,   155,
      76,   153,    75,   151,    75,   153,   158,   158,    23,   109,
     110,   111,   156,   156,   152,    74,    75,   111,    77,   156,
     139,   112,    77,    77,    24,    41,    57,   113,   114,    63,
      74,   155,   153,    38,   114,   121,   122,    76,    25,    30,
     115,   116,    74,    75,   122,   156,    74,    74,    75,   116,
     123,   156,    26,    27,    28,    29,   117,   118,    31,    32,
      33,    34,   119,   120,    41,    57,   124,   125,   156,   154,
     154,   154,   154,    75,   118,   154,   154,   154,   154,    75,
     120,   155,   153,    75,   125,    77,    64,    76,   156,   156,
     156,    77,    75
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
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, yyvsp[0]._string );
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, yyvsp[0]._string );
     ;}
    break;

  case 9:

    { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, yyvsp[0]._connectionType ); 
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, yyvsp[0]._unsigned );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, yyvsp[0]._unsigned );
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

    { flags = eq::Frame::BUFFER_UNDEFINED;;}
    break;

  case 125:

    { eqCompound->setBuffers( flags ); flags = 0; ;}
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

    { flags |= eq::Frame::BUFFER_COLOR; ;}
    break;

  case 148:

    { flags |= eq::Frame::BUFFER_DEPTH; ;}
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

    { frame->setViewport(eq::Viewport( yyvsp[0]._viewport[0], yyvsp[0]._viewport[1], yyvsp[0]._viewport[2], yyvsp[0]._viewport[3])); ;}
    break;

  case 165:

    { flags = eq::Frame::BUFFER_UNDEFINED; ;}
    break;

  case 166:

    { frame->setBuffers( flags ); flags = 0; ;}
    break;

  case 167:

    { 
         yyval._viewport[0] = yyvsp[-4]._float;
         yyval._viewport[1] = yyvsp[-3]._float;
         yyval._viewport[2] = yyvsp[-2]._float;
         yyval._viewport[3] = yyvsp[-1]._float;
     ;}
    break;

  case 168:

    { yyval._int = eq::ON; ;}
    break;

  case 169:

    { yyval._int = eq::OFF; ;}
    break;

  case 170:

    { yyval._int = eq::AUTO; ;}
    break;

  case 171:

    { yyval._int = yyvsp[0]._int; ;}
    break;

  case 172:

    {
         stringBuf = yytext;
         yyval._string = stringBuf.c_str(); 
     ;}
    break;

  case 173:

    { yyval._float = atof( yytext ); ;}
    break;

  case 174:

    { yyval._float = yyvsp[0]._int; ;}
    break;

  case 175:

    { yyval._int = atoi( yytext ); ;}
    break;

  case 176:

    { yyval._int = yyvsp[0]._unsigned; ;}
    break;

  case 177:

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

