/* A Bison parser, made by GNU Bison 2.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.2"

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
     EQTOKEN_CONNECTION_TYPE = 259,
     EQTOKEN_CONNECTION_HOSTNAME = 260,
     EQTOKEN_CONNECTION_TCPIP_PORT = 261,
     EQTOKEN_CONNECTION_LAUNCH_TIMEOUT = 262,
     EQTOKEN_CONNECTION_LAUNCH_COMMAND = 263,
     EQTOKEN_WINDOW_IATTR_HINTS_STEREO = 264,
     EQTOKEN_CONFIG_FATTR_EYE_BASE = 265,
     EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER = 266,
     EQTOKEN_WINDOW_IATTR_PLANES_COLOR = 267,
     EQTOKEN_WINDOW_IATTR_PLANES_ALPHA = 268,
     EQTOKEN_WINDOW_IATTR_PLANES_DEPTH = 269,
     EQTOKEN_WINDOW_IATTR_PLANES_STENCIL = 270,
     EQTOKEN_SERVER = 271,
     EQTOKEN_CONFIG = 272,
     EQTOKEN_APPNODE = 273,
     EQTOKEN_NODE = 274,
     EQTOKEN_PIPE = 275,
     EQTOKEN_WINDOW = 276,
     EQTOKEN_ATTRIBUTES = 277,
     EQTOKEN_HINTS = 278,
     EQTOKEN_STEREO = 279,
     EQTOKEN_DOUBLEBUFFER = 280,
     EQTOKEN_PLANES = 281,
     EQTOKEN_COLOR = 282,
     EQTOKEN_ALPHA = 283,
     EQTOKEN_DEPTH = 284,
     EQTOKEN_STENCIL = 285,
     EQTOKEN_ON = 286,
     EQTOKEN_OFF = 287,
     EQTOKEN_AUTO = 288,
     EQTOKEN_CHANNEL = 289,
     EQTOKEN_COMPOUND = 290,
     EQTOKEN_CONNECTION = 291,
     EQTOKEN_NAME = 292,
     EQTOKEN_TYPE = 293,
     EQTOKEN_TCPIP = 294,
     EQTOKEN_HOSTNAME = 295,
     EQTOKEN_COMMAND = 296,
     EQTOKEN_TIMEOUT = 297,
     EQTOKEN_TASK = 298,
     EQTOKEN_EYE = 299,
     EQTOKEN_EYE_BASE = 300,
     EQTOKEN_FORMAT = 301,
     EQTOKEN_CLEAR = 302,
     EQTOKEN_DRAW = 303,
     EQTOKEN_READBACK = 304,
     EQTOKEN_CYCLOP = 305,
     EQTOKEN_LEFT = 306,
     EQTOKEN_RIGHT = 307,
     EQTOKEN_VIEWPORT = 308,
     EQTOKEN_RANGE = 309,
     EQTOKEN_DISPLAY = 310,
     EQTOKEN_WALL = 311,
     EQTOKEN_BOTTOM_LEFT = 312,
     EQTOKEN_BOTTOM_RIGHT = 313,
     EQTOKEN_TOP_LEFT = 314,
     EQTOKEN_SYNC = 315,
     EQTOKEN_LATENCY = 316,
     EQTOKEN_SWAPBARRIER = 317,
     EQTOKEN_OUTPUTFRAME = 318,
     EQTOKEN_INPUTFRAME = 319,
     EQTOKEN_STRING = 320,
     EQTOKEN_FLOAT = 321,
     EQTOKEN_INTEGER = 322,
     EQTOKEN_UNSIGNED = 323
   };
#endif
/* Tokens.  */
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_TYPE 259
#define EQTOKEN_CONNECTION_HOSTNAME 260
#define EQTOKEN_CONNECTION_TCPIP_PORT 261
#define EQTOKEN_CONNECTION_LAUNCH_TIMEOUT 262
#define EQTOKEN_CONNECTION_LAUNCH_COMMAND 263
#define EQTOKEN_WINDOW_IATTR_HINTS_STEREO 264
#define EQTOKEN_CONFIG_FATTR_EYE_BASE 265
#define EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER 266
#define EQTOKEN_WINDOW_IATTR_PLANES_COLOR 267
#define EQTOKEN_WINDOW_IATTR_PLANES_ALPHA 268
#define EQTOKEN_WINDOW_IATTR_PLANES_DEPTH 269
#define EQTOKEN_WINDOW_IATTR_PLANES_STENCIL 270
#define EQTOKEN_SERVER 271
#define EQTOKEN_CONFIG 272
#define EQTOKEN_APPNODE 273
#define EQTOKEN_NODE 274
#define EQTOKEN_PIPE 275
#define EQTOKEN_WINDOW 276
#define EQTOKEN_ATTRIBUTES 277
#define EQTOKEN_HINTS 278
#define EQTOKEN_STEREO 279
#define EQTOKEN_DOUBLEBUFFER 280
#define EQTOKEN_PLANES 281
#define EQTOKEN_COLOR 282
#define EQTOKEN_ALPHA 283
#define EQTOKEN_DEPTH 284
#define EQTOKEN_STENCIL 285
#define EQTOKEN_ON 286
#define EQTOKEN_OFF 287
#define EQTOKEN_AUTO 288
#define EQTOKEN_CHANNEL 289
#define EQTOKEN_COMPOUND 290
#define EQTOKEN_CONNECTION 291
#define EQTOKEN_NAME 292
#define EQTOKEN_TYPE 293
#define EQTOKEN_TCPIP 294
#define EQTOKEN_HOSTNAME 295
#define EQTOKEN_COMMAND 296
#define EQTOKEN_TIMEOUT 297
#define EQTOKEN_TASK 298
#define EQTOKEN_EYE 299
#define EQTOKEN_EYE_BASE 300
#define EQTOKEN_FORMAT 301
#define EQTOKEN_CLEAR 302
#define EQTOKEN_DRAW 303
#define EQTOKEN_READBACK 304
#define EQTOKEN_CYCLOP 305
#define EQTOKEN_LEFT 306
#define EQTOKEN_RIGHT 307
#define EQTOKEN_VIEWPORT 308
#define EQTOKEN_RANGE 309
#define EQTOKEN_DISPLAY 310
#define EQTOKEN_WALL 311
#define EQTOKEN_BOTTOM_LEFT 312
#define EQTOKEN_BOTTOM_RIGHT 313
#define EQTOKEN_TOP_LEFT 314
#define EQTOKEN_SYNC 315
#define EQTOKEN_LATENCY 316
#define EQTOKEN_SWAPBARRIER 317
#define EQTOKEN_OUTPUTFRAME 318
#define EQTOKEN_INPUTFRAME 319
#define EQTOKEN_STRING 320
#define EQTOKEN_FLOAT 321
#define EQTOKEN_INTEGER 322
#define EQTOKEN_UNSIGNED 323




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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE

{
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
}
/* Line 193 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */


#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   303

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  80
/* YYNRULES -- Number of rules.  */
#define YYNRULES  169
/* YYNRULES -- Number of states.  */
#define YYNSTATES  286

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
       2,    71,     2,    72,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,     2,    70,     2,     2,     2,     2,
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
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    38,    41,    44,    47,    50,    53,
      55,    56,    62,    64,    67,    68,    76,    77,    79,    82,
      85,    90,    91,    93,    96,    99,   101,   104,   106,   108,
     109,   117,   118,   126,   127,   129,   132,   133,   134,   136,
     139,   140,   146,   147,   149,   152,   155,   158,   161,   164,
     166,   169,   170,   177,   178,   180,   183,   186,   189,   191,
     194,   195,   202,   203,   205,   208,   213,   216,   219,   220,
     222,   225,   230,   235,   236,   238,   241,   244,   247,   248,
     250,   253,   256,   259,   262,   265,   267,   270,   271,   277,
     278,   280,   283,   286,   289,   291,   294,   295,   303,   304,
     306,   307,   309,   312,   315,   318,   319,   325,   326,   332,
     333,   339,   342,   348,   350,   352,   354,   356,   357,   359,
     362,   364,   366,   368,   369,   371,   374,   376,   378,   380,
     381,   383,   386,   388,   390,   412,   413,   419,   420,   422,
     425,   428,   429,   435,   436,   442,   443,   445,   448,   451,
     458,   460,   462,   464,   466,   468,   470,   472,   474,   476
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      74,     0,    -1,    75,    78,    -1,     3,    69,    76,    70,
      -1,    -1,    75,    -1,    76,    75,    -1,     4,    77,    -1,
       5,   149,    -1,     6,   152,    -1,     7,   152,    -1,     8,
     149,    -1,     9,   148,    -1,    11,   148,    -1,    12,   148,
      -1,    13,   148,    -1,    14,   148,    -1,    15,   148,    -1,
      10,   150,    -1,    39,    -1,    -1,    16,    69,    79,    80,
      70,    -1,    81,    -1,    80,    81,    -1,    -1,    17,    69,
      82,    83,    87,   121,    70,    -1,    -1,    84,    -1,    83,
      84,    -1,    61,   152,    -1,    22,    69,    85,    70,    -1,
      -1,    86,    -1,    85,    86,    -1,    45,   150,    -1,    88,
      -1,    87,    88,    -1,    91,    -1,    89,    -1,    -1,    19,
      69,    90,    95,    93,   100,    70,    -1,    -1,    18,    69,
      92,    95,    93,   100,    70,    -1,    -1,    94,    -1,    93,
      94,    -1,    -1,    -1,    96,    -1,    95,    96,    -1,    -1,
      36,    69,    97,    98,    70,    -1,    -1,    99,    -1,    98,
      99,    -1,    38,    77,    -1,    40,   149,    -1,    41,   149,
      -1,    42,   152,    -1,   101,    -1,   100,   101,    -1,    -1,
      20,    69,   102,   103,   105,    70,    -1,    -1,   104,    -1,
     103,   104,    -1,    55,   152,    -1,    53,   147,    -1,   106,
      -1,   105,   106,    -1,    -1,    21,    69,   107,   108,   116,
      70,    -1,    -1,   109,    -1,   108,   109,    -1,    22,    69,
     110,    70,    -1,    37,   149,    -1,    53,   147,    -1,    -1,
     111,    -1,   110,   111,    -1,    23,    69,   112,    70,    -1,
      26,    69,   114,    70,    -1,    -1,   113,    -1,   112,   113,
      -1,    24,   148,    -1,    25,   148,    -1,    -1,   115,    -1,
     114,   115,    -1,    27,   148,    -1,    28,   148,    -1,    29,
     148,    -1,    30,   148,    -1,   117,    -1,   116,   117,    -1,
      -1,    34,    69,   118,   119,    70,    -1,    -1,   120,    -1,
     119,   120,    -1,    37,   149,    -1,    53,   147,    -1,   122,
      -1,   121,   122,    -1,    -1,    35,    69,   123,   125,   124,
     125,    70,    -1,    -1,   121,    -1,    -1,   126,    -1,   125,
     126,    -1,    37,   149,    -1,    34,   149,    -1,    -1,    43,
      71,   127,   130,    72,    -1,    -1,    44,    71,   128,   132,
      72,    -1,    -1,    46,    71,   129,   134,    72,    -1,    53,
     147,    -1,    54,    71,   150,   150,    72,    -1,   136,    -1,
     137,    -1,   141,    -1,   143,    -1,    -1,   131,    -1,   130,
     131,    -1,    47,    -1,    48,    -1,    49,    -1,    -1,   133,
      -1,   132,   133,    -1,    50,    -1,    51,    -1,    52,    -1,
      -1,   135,    -1,   134,   135,    -1,    27,    -1,    29,    -1,
      56,    69,    57,    71,   150,   150,   150,    72,    58,    71,
     150,   150,   150,    72,    59,    71,   150,   150,   150,    72,
      70,    -1,    -1,    62,    69,   138,   139,    70,    -1,    -1,
     140,    -1,   139,   140,    -1,    37,   149,    -1,    -1,    63,
      69,   142,   145,    70,    -1,    -1,    64,    69,   144,   145,
      70,    -1,    -1,   146,    -1,   145,   146,    -1,    37,   149,
      -1,    71,   150,   150,   150,   150,    72,    -1,    31,    -1,
      32,    -1,    33,    -1,   151,    -1,    65,    -1,    66,    -1,
     151,    -1,    67,    -1,   152,    -1,    68,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   138,   138,   140,   141,   144,   144,   147,   152,   157,
     162,   167,   172,   177,   182,   187,   192,   197,   202,   208,
     210,   210,   213,   213,   214,   214,   217,   217,   217,   219,
     220,   221,   221,   221,   223,   226,   226,   227,   227,   228,
     228,   232,   232,   236,   236,   236,   237,   240,   244,   244,
     246,   245,   252,   252,   253,   255,   256,   257,   258,   261,
     261,   262,   262,   265,   265,   265,   267,   268,   274,   274,
     275,   275,   278,   278,   278,   280,   282,   283,   291,   291,
     291,   293,   294,   295,   295,   295,   297,   299,   301,   301,
     301,   303,   305,   307,   309,   312,   312,   313,   313,   316,
     317,   317,   319,   320,   330,   330,   332,   331,   345,   345,
     347,   347,   348,   350,   351,   359,   359,   361,   361,   363,
     363,   365,   367,   369,   370,   371,   372,   374,   374,   374,
     376,   377,   378,   380,   380,   380,   382,   383,   384,   386,
     386,   386,   388,   389,   391,   412,   412,   418,   418,   419,
     421,   423,   423,   429,   429,   435,   435,   435,   437,   439,
     448,   449,   450,   451,   453,   458,   459,   461,   462,   463
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQTOKEN_GLOBAL",
  "EQTOKEN_CONNECTION_TYPE", "EQTOKEN_CONNECTION_HOSTNAME",
  "EQTOKEN_CONNECTION_TCPIP_PORT", "EQTOKEN_CONNECTION_LAUNCH_TIMEOUT",
  "EQTOKEN_CONNECTION_LAUNCH_COMMAND", "EQTOKEN_WINDOW_IATTR_HINTS_STEREO",
  "EQTOKEN_CONFIG_FATTR_EYE_BASE",
  "EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER",
  "EQTOKEN_WINDOW_IATTR_PLANES_COLOR", "EQTOKEN_WINDOW_IATTR_PLANES_ALPHA",
  "EQTOKEN_WINDOW_IATTR_PLANES_DEPTH",
  "EQTOKEN_WINDOW_IATTR_PLANES_STENCIL", "EQTOKEN_SERVER",
  "EQTOKEN_CONFIG", "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE",
  "EQTOKEN_WINDOW", "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINTS",
  "EQTOKEN_STEREO", "EQTOKEN_DOUBLEBUFFER", "EQTOKEN_PLANES",
  "EQTOKEN_COLOR", "EQTOKEN_ALPHA", "EQTOKEN_DEPTH", "EQTOKEN_STENCIL",
  "EQTOKEN_ON", "EQTOKEN_OFF", "EQTOKEN_AUTO", "EQTOKEN_CHANNEL",
  "EQTOKEN_COMPOUND", "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE",
  "EQTOKEN_TCPIP", "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND",
  "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_EYE_BASE",
  "EQTOKEN_FORMAT", "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_READBACK",
  "EQTOKEN_CYCLOP", "EQTOKEN_LEFT", "EQTOKEN_RIGHT", "EQTOKEN_VIEWPORT",
  "EQTOKEN_RANGE", "EQTOKEN_DISPLAY", "EQTOKEN_WALL",
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
  "compoundEye", "compoundFormats", "compoundFormat", "wall",
  "swapBarrier", "@13", "swapBarrierFields", "swapBarrierField",
  "outputFrame", "@14", "inputFrame", "@15", "frameFields", "frameField",
  "viewport", "IATTR", "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   123,
     125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    73,    74,    75,    75,    76,    76,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    77,
      79,    78,    80,    80,    82,    81,    83,    83,    83,    84,
      84,    85,    85,    85,    86,    87,    87,    88,    88,    90,
      89,    92,    91,    93,    93,    93,    94,    95,    95,    95,
      97,    96,    98,    98,    98,    99,    99,    99,    99,   100,
     100,   102,   101,   103,   103,   103,   104,   104,   105,   105,
     107,   106,   108,   108,   108,   109,   109,   109,   110,   110,
     110,   111,   111,   112,   112,   112,   113,   113,   114,   114,
     114,   115,   115,   115,   115,   116,   116,   118,   117,   119,
     119,   119,   120,   120,   121,   121,   123,   122,   124,   124,
     125,   125,   125,   126,   126,   127,   126,   128,   126,   129,
     126,   126,   126,   126,   126,   126,   126,   130,   130,   130,
     131,   131,   131,   132,   132,   132,   133,   133,   133,   134,
     134,   134,   135,   135,   136,   138,   137,   139,   139,   139,
     140,   142,   141,   144,   143,   145,   145,   145,   146,   147,
     148,   148,   148,   148,   149,   150,   150,   151,   151,   152
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       0,     5,     1,     2,     0,     7,     0,     1,     2,     2,
       4,     0,     1,     2,     2,     1,     2,     1,     1,     0,
       7,     0,     7,     0,     1,     2,     0,     0,     1,     2,
       0,     5,     0,     1,     2,     2,     2,     2,     2,     1,
       2,     0,     6,     0,     1,     2,     2,     2,     1,     2,
       0,     6,     0,     1,     2,     4,     2,     2,     0,     1,
       2,     4,     4,     0,     1,     2,     2,     2,     0,     1,
       2,     2,     2,     2,     2,     1,     2,     0,     5,     0,
       1,     2,     2,     2,     1,     2,     0,     7,     0,     1,
       0,     1,     2,     2,     2,     0,     5,     0,     5,     0,
       5,     2,     5,     1,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     0,
       1,     2,     1,     1,    21,     0,     5,     0,     1,     2,
       2,     0,     5,     0,     5,     0,     1,     2,     2,     6,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,    19,     7,   164,
       8,   169,     9,    10,    11,   160,   161,   162,   167,    12,
     163,   168,   165,    18,   166,    13,    14,    15,    16,    17,
       1,     0,     2,     5,     0,    20,     3,     6,     0,     0,
       0,    22,    24,    21,    23,    26,     0,     0,     0,    27,
      31,    29,     0,     0,    28,     0,    35,    38,    37,     0,
       0,    32,    41,    39,     0,    36,     0,   104,    34,    30,
      33,    47,    47,   106,    25,   105,     0,    43,    48,    43,
     110,    50,     0,    44,    49,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   108,   111,   123,
     124,   125,   126,    52,     0,    45,     0,    59,     0,   114,
     113,   115,   117,   119,     0,   121,     0,     0,   145,   151,
     153,   109,   110,   112,     0,     0,     0,     0,     0,    53,
      61,    42,    60,    40,   127,   133,   139,     0,     0,     0,
     147,   155,   155,     0,    55,    56,    57,    58,    51,    54,
      63,   130,   131,   132,     0,   128,   136,   137,   138,     0,
     134,   142,   143,     0,   140,     0,     0,     0,     0,     0,
     148,     0,     0,   156,     0,   107,     0,     0,     0,    64,
     116,   129,   118,   135,   120,   141,     0,   122,     0,   150,
     146,   149,   158,   152,   157,   154,    67,    66,     0,    65,
       0,    68,     0,     0,    70,    62,    69,   159,     0,    72,
       0,     0,     0,     0,     0,    73,     0,    78,    76,    77,
       0,    74,     0,    95,     0,     0,     0,     0,    79,    97,
      71,    96,     0,    83,    88,    75,    80,    99,     0,     0,
       0,     0,    84,     0,     0,     0,     0,     0,    89,     0,
       0,     0,   100,     0,    86,    87,    81,    85,    91,    92,
      93,    94,    82,    90,   102,   103,    98,   101,     0,     0,
       0,     0,     0,     0,     0,   144
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    14,    15,    44,    18,    42,    48,    50,    51,    55,
      58,    59,    70,    71,    65,    66,    67,    82,    68,    81,
      92,    93,    87,    88,   113,   138,   139,   116,   117,   160,
     188,   189,   210,   211,   219,   224,   225,   237,   238,   251,
     252,   257,   258,   232,   233,   247,   261,   262,    76,    77,
      90,   132,   107,   108,   144,   145,   146,   164,   165,   169,
     170,   173,   174,   109,   110,   150,   179,   180,   111,   151,
     112,   152,   182,   183,   125,    29,    20,    33,    34,    31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -186
static const yytype_int16 yypact[] =
{
     221,   -21,    21,    -4,    45,    45,    -4,    36,   143,    36,
      36,    36,    36,    36,   100,    62,   221,  -186,  -186,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,    59,  -186,  -186,    16,  -186,  -186,  -186,   132,    71,
      15,  -186,  -186,  -186,  -186,    33,    85,    45,    32,  -186,
      56,  -186,   109,   115,  -186,    38,  -186,  -186,  -186,   143,
       1,  -186,  -186,  -186,   136,  -186,    10,  -186,  -186,  -186,
    -186,   119,   119,  -186,  -186,  -186,   149,   119,  -186,   119,
     123,  -186,    94,  -186,  -186,    94,    -4,    -4,   121,   167,
     169,   171,   172,   177,   178,   183,   187,   127,  -186,  -186,
    -186,  -186,  -186,   166,   188,  -186,    -8,  -186,    13,  -186,
    -186,  -186,  -186,  -186,   143,  -186,   143,   111,  -186,  -186,
    -186,   206,   123,  -186,    21,    -4,    -4,    45,    78,  -186,
    -186,  -186,  -186,  -186,    98,   162,    97,   143,   143,   191,
     226,   227,   227,    88,  -186,  -186,  -186,  -186,  -186,  -186,
      80,  -186,  -186,  -186,   147,  -186,  -186,  -186,  -186,   151,
    -186,  -186,  -186,    20,  -186,   143,   193,   143,    -4,    37,
    -186,    -4,    39,  -186,    40,  -186,   171,    45,    44,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,   143,  -186,   143,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,   197,  -186,
      14,  -186,   195,   143,  -186,  -186,  -186,  -186,   196,    84,
     211,   201,    -4,   171,    74,  -186,   200,    79,  -186,  -186,
     203,  -186,     2,  -186,   143,   204,   205,    11,  -186,  -186,
    -186,  -186,   143,    28,   170,  -186,  -186,    86,   143,    36,
      36,    19,  -186,    36,    36,    36,    36,    12,  -186,    -4,
     171,    42,  -186,   207,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,   216,   209,
     143,   143,   143,   210,   208,  -186
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -186,  -186,    54,  -186,   142,  -186,  -186,  -186,   231,  -186,
    -186,   219,  -186,   213,  -186,   220,  -186,  -186,  -186,  -186,
     198,    64,   202,    76,  -186,  -186,   148,   194,    99,  -186,
    -186,   102,  -186,    81,  -186,  -186,    68,  -186,    51,  -186,
      43,  -186,    41,  -186,    61,  -186,  -186,    34,   189,   -73,
    -186,  -186,   165,   -94,  -186,  -186,  -186,  -186,   135,  -186,
     131,  -186,   128,  -186,  -186,  -186,  -186,   124,  -186,  -186,
    -186,  -186,   150,    55,  -185,     5,    -6,   -60,    -5,     6
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      24,   206,    30,    85,    30,    30,    30,    30,    30,    78,
      22,    23,   114,   133,    35,    36,    37,    38,    39,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    49,   114,   235,   208,   230,   236,   229,   253,
     254,   255,   256,   249,   250,    74,    69,   171,    16,   172,
      62,    63,   249,   250,    56,    56,    62,    63,    85,   133,
      17,    19,   141,    61,   147,   208,   148,    25,    26,    27,
      43,    79,   240,    74,   178,   275,   181,   181,    41,   259,
      84,   245,   272,   143,   215,    53,    46,   175,   176,   266,
     119,   120,   194,    57,    57,   260,   221,   186,    47,   187,
      40,    69,   235,    28,    21,   236,   221,   200,   230,   203,
     205,   222,   276,    21,   114,   196,   134,   198,   135,   136,
     137,   222,    96,   259,   171,    97,   172,   223,    45,   155,
     156,    98,    99,   186,   100,   187,   212,   223,   213,   260,
      52,   101,   102,   157,   103,   161,   162,   163,   158,    49,
     104,   105,   106,   218,    60,    86,   115,    96,   185,   115,
      97,    96,    74,    94,    97,    94,    98,    99,   149,   100,
      98,    99,   199,   100,   242,   202,   101,   102,    72,   103,
     101,   102,   248,   103,    73,   104,   105,   106,   263,   104,
     105,   106,   121,   207,   161,   162,   163,   253,   254,   255,
     256,   166,   167,   168,   134,    83,   135,   136,   137,    32,
      28,    21,   166,   167,   168,   142,   228,   142,    91,   190,
     281,   282,   283,   192,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,   204,   122,   204,
     123,    74,   124,   126,    30,    30,   127,   128,    30,    30,
      30,    30,   129,   274,   264,   265,   130,   140,   268,   269,
     270,   271,   177,   178,   181,   197,   214,   217,   220,   226,
     227,   234,   239,   243,   244,   279,   154,    64,   285,   278,
     280,    54,   284,    80,    89,    75,   159,    95,   246,   118,
     209,   216,   231,   241,   267,   277,   131,   153,   273,   191,
     193,   195,   184,   201
};

static const yytype_uint16 yycheck[] =
{
       6,   186,     7,    76,     9,    10,    11,    12,    13,    69,
       4,     5,    20,   107,     9,    10,    11,    12,    13,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    17,    20,    23,    21,    34,    26,   223,    27,
      28,    29,    30,    24,    25,    35,    45,    27,    69,    29,
      18,    19,    24,    25,    22,    22,    18,    19,   131,   153,
      39,    65,    70,    57,   124,    21,   126,    31,    32,    33,
      16,    70,    70,    35,    37,   260,    37,    37,    16,    37,
      70,    70,    70,    70,    70,    70,    70,   147,   148,    70,
      96,    97,    72,    61,    61,    53,    22,    53,    44,    55,
       0,    45,    23,    67,    68,    26,    22,    70,    34,    70,
      70,    37,    70,    68,    20,   175,    38,   177,    40,    41,
      42,    37,    34,    37,    27,    37,    29,    53,    69,   135,
     136,    43,    44,    53,    46,    55,   196,    53,   198,    53,
      69,    53,    54,   137,    56,    47,    48,    49,    70,    17,
      62,    63,    64,   213,    69,    36,    92,    34,    70,    95,
      37,    34,    35,    87,    37,    89,    43,    44,    57,    46,
      43,    44,   178,    46,   234,   181,    53,    54,    69,    56,
      53,    54,   242,    56,    69,    62,    63,    64,   248,    62,
      63,    64,    71,   187,    47,    48,    49,    27,    28,    29,
      30,    50,    51,    52,    38,    69,    40,    41,    42,    66,
      67,    68,    50,    51,    52,   116,   222,   118,    69,    72,
     280,   281,   282,    72,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,   182,    71,   184,
      71,    35,    71,    71,   249,   250,    69,    69,   253,   254,
     255,   256,    69,   259,   249,   250,    69,    69,   253,   254,
     255,   256,    71,    37,    37,    72,    69,    72,    72,    58,
      69,    71,    69,    69,    69,    59,   134,    58,    70,    72,
      71,    50,    72,    70,    82,    65,   138,    89,   237,    95,
     188,   210,   224,   232,   251,   261,   107,   132,   257,   164,
     169,   173,   152,   179
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    74,    75,    69,    39,    77,    65,
     149,    68,   152,   152,   149,    31,    32,    33,    67,   148,
     151,   152,    66,   150,   151,   148,   148,   148,   148,   148,
       0,    16,    78,    75,    76,    69,    70,    75,    79,    17,
      80,    81,    69,    70,    81,    82,    22,    61,    83,    84,
      69,   152,    18,    19,    84,    87,    88,    89,    91,    45,
      85,    86,    69,    69,    35,    88,   121,   122,   150,    70,
      86,    92,    90,    69,    70,   122,    36,    95,    96,    95,
     123,    69,    93,    94,    96,    93,    34,    37,    43,    44,
      46,    53,    54,    56,    62,    63,    64,   125,   126,   136,
     137,   141,   143,    97,    20,    94,   100,   101,   100,   149,
     149,    71,    71,    71,    71,   147,    71,    69,    69,    69,
      69,   121,   124,   126,    38,    40,    41,    42,    98,    99,
      69,    70,   101,    70,   127,   128,   129,   150,   150,    57,
     138,   142,   144,   125,    77,   149,   149,   152,    70,    99,
     102,    47,    48,    49,   130,   131,    50,    51,    52,   132,
     133,    27,    29,   134,   135,   150,   150,    71,    37,   139,
     140,    37,   145,   146,   145,    70,    53,    55,   103,   104,
      72,   131,    72,   133,    72,   135,   150,    72,   150,   149,
      70,   140,   149,    70,   146,    70,   147,   152,    21,   104,
     105,   106,   150,   150,    69,    70,   106,    72,   150,   107,
      72,    22,    37,    53,   108,   109,    58,    69,   149,   147,
      34,   109,   116,   117,    71,    23,    26,   110,   111,    69,
      70,   117,   150,    69,    69,    70,   111,   118,   150,    24,
      25,   112,   113,    27,    28,    29,    30,   114,   115,    37,
      53,   119,   120,   150,   148,   148,    70,   113,   148,   148,
     148,   148,    70,   115,   149,   147,    70,   120,    72,    59,
      71,   150,   150,   150,    72,    70
};

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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
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
    while (YYID (0))
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, const YYSTYPE * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    const YYSTYPE * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, const YYSTYPE * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    const YYSTYPE * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, 
		   int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule
		   )
    YYSTYPE *yyvsp;
    
		   int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  YYUSE (yyvaluep);

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
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
             eqs::ConnectionDescription::IATTR_TYPE, (yyvsp[(2) - (2)]._connectionType) ); 
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, (yyvsp[(2) - (2)]._string) );
     ;}
    break;

  case 9:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, (yyvsp[(2) - (2)]._unsigned) );
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, (yyvsp[(2) - (2)]._unsigned) );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, (yyvsp[(2) - (2)]._string) );
     ;}
    break;

  case 12:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_STEREO, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 13:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINTS_DOUBLEBUFFER, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 14:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 15:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 16:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 17:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 18:

    {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, (yyvsp[(2) - (2)]._float) );
     ;}
    break;

  case 19:

    { (yyval._connectionType) = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 20:

    { server = loader->createServer(); ;}
    break;

  case 24:

    { config = loader->createConfig(); ;}
    break;

  case 25:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 29:

    { config->setLatency( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 34:

    { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, (yyvsp[(2) - (2)]._float) ); ;}
    break;

  case 39:

    { node = loader->createNode(); ;}
    break;

  case 40:

    { config->addNode( node ); node = NULL; ;}
    break;

  case 41:

    { node = loader->createNode(); ;}
    break;

  case 42:

    { config->addApplicationNode( node ); node = NULL; ;}
    break;

  case 47:

    { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             ;}
    break;

  case 50:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 51:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             ;}
    break;

  case 55:

    { connectionDescription->type = (yyvsp[(2) - (2)]._connectionType); ;}
    break;

  case 56:

    { connectionDescription->hostname = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 57:

    { connectionDescription->launchCommand = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 58:

    { connectionDescription->launchTimeout = (yyvsp[(2) - (2)]._unsigned); ;}
    break;

  case 61:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 62:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 66:

    { eqPipe->setDisplay( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 67:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], (int)(yyvsp[(2) - (2)]._viewport)[1],
                                                      (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
        ;}
    break;

  case 70:

    { window = loader->createWindow(); ;}
    break;

  case 71:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 76:

    { window->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 77:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], 
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3])); 
        ;}
    break;

  case 86:

    { window->setIAttribute( eq::Window::IATTR_HINTS_STEREO, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 87:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DOUBLEBUFFER, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 91:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 92:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 93:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 94:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 97:

    { channel = loader->createChannel(); ;}
    break;

  case 98:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 102:

    { channel->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 103:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0],
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3]));
        ;}
    break;

  case 106:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;}
    break;

  case 107:

    { eqCompound = eqCompound->getParent(); ;}
    break;

  case 113:

    { eqCompound->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 114:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[(2) - (2)]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;}
    break;

  case 115:

    { eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 117:

    { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED ); ;}
    break;

  case 119:

    { eqCompound->setFormats( eq::Frame::FORMAT_UNDEFINED );;}
    break;

  case 121:

    { eqCompound->setViewport( eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3] )); ;}
    break;

  case 122:

    { eqCompound->setRange( eq::Range( (yyvsp[(3) - (5)]._float), (yyvsp[(4) - (5)]._float) )); ;}
    break;

  case 130:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 131:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 132:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK ); ;}
    break;

  case 136:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 137:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 138:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 142:

    { eqCompound->enableFormat( eq::Frame::FORMAT_COLOR ); ;}
    break;

  case 143:

    { eqCompound->enableFormat( eq::Frame::FORMAT_DEPTH ); ;}
    break;

  case 144:

    { 
        eq::Wall wall;
        wall.bottomLeft[0] = (yyvsp[(5) - (21)]._float);
        wall.bottomLeft[1] = (yyvsp[(6) - (21)]._float);
        wall.bottomLeft[2] = (yyvsp[(7) - (21)]._float);

        wall.bottomRight[0] = (yyvsp[(11) - (21)]._float);
        wall.bottomRight[1] = (yyvsp[(12) - (21)]._float);
        wall.bottomRight[2] = (yyvsp[(13) - (21)]._float);

        wall.topLeft[0] = (yyvsp[(17) - (21)]._float);
        wall.topLeft[1] = (yyvsp[(18) - (21)]._float);
        wall.topLeft[2] = (yyvsp[(19) - (21)]._float);
        eqCompound->setWall( wall );
    ;}
    break;

  case 145:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 146:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 150:

    { swapBarrier->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 151:

    { frame = new eqs::Frame(); ;}
    break;

  case 152:

    { 
            eqCompound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 153:

    { frame = new eqs::Frame(); ;}
    break;

  case 154:

    { 
            eqCompound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 158:

    { frame->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 159:

    { 
         (yyval._viewport)[0] = (yyvsp[(2) - (6)]._float);
         (yyval._viewport)[1] = (yyvsp[(3) - (6)]._float);
         (yyval._viewport)[2] = (yyvsp[(4) - (6)]._float);
         (yyval._viewport)[3] = (yyvsp[(5) - (6)]._float);
     ;}
    break;

  case 160:

    { (yyval._int) = eq::ON; ;}
    break;

  case 161:

    { (yyval._int) = eq::OFF; ;}
    break;

  case 162:

    { (yyval._int) = eq::AUTO; ;}
    break;

  case 163:

    { (yyval._int) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 164:

    {
         stringBuf = yytext;
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 165:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 166:

    { (yyval._float) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 167:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 168:

    { (yyval._int) = (yyvsp[(1) - (1)]._unsigned); ;}
    break;

  case 169:

    { (yyval._unsigned) = atoi( yytext ); ;}
    break;


/* Line 1267 of yacc.c.  */

      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
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
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
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

