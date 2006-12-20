/* A Bison parser, made by GNU Bison 2.3.  */

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
#define YYBISON_VERSION "2.3"

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
     EQTOKEN_WINDOW_IATTR_HINT_STEREO = 265,
     EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER = 266,
     EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN = 267,
     EQTOKEN_WINDOW_IATTR_HINT_DECORATION = 268,
     EQTOKEN_WINDOW_IATTR_PLANES_COLOR = 269,
     EQTOKEN_WINDOW_IATTR_PLANES_ALPHA = 270,
     EQTOKEN_WINDOW_IATTR_PLANES_DEPTH = 271,
     EQTOKEN_WINDOW_IATTR_PLANES_STENCIL = 272,
     EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS = 273,
     EQTOKEN_SERVER = 274,
     EQTOKEN_CONFIG = 275,
     EQTOKEN_APPNODE = 276,
     EQTOKEN_NODE = 277,
     EQTOKEN_PIPE = 278,
     EQTOKEN_WINDOW = 279,
     EQTOKEN_ATTRIBUTES = 280,
     EQTOKEN_HINT_STEREO = 281,
     EQTOKEN_HINT_DOUBLEBUFFER = 282,
     EQTOKEN_HINT_FULLSCREEN = 283,
     EQTOKEN_HINT_DECORATION = 284,
     EQTOKEN_HINT_STATISTICS = 285,
     EQTOKEN_PLANES_COLOR = 286,
     EQTOKEN_PLANES_ALPHA = 287,
     EQTOKEN_PLANES_DEPTH = 288,
     EQTOKEN_PLANES_STENCIL = 289,
     EQTOKEN_ON = 290,
     EQTOKEN_OFF = 291,
     EQTOKEN_AUTO = 292,
     EQTOKEN_FASTEST = 293,
     EQTOKEN_NICEST = 294,
     EQTOKEN_CHANNEL = 295,
     EQTOKEN_COMPOUND = 296,
     EQTOKEN_CONNECTION = 297,
     EQTOKEN_NAME = 298,
     EQTOKEN_TYPE = 299,
     EQTOKEN_TCPIP = 300,
     EQTOKEN_HOSTNAME = 301,
     EQTOKEN_COMMAND = 302,
     EQTOKEN_TIMEOUT = 303,
     EQTOKEN_TASK = 304,
     EQTOKEN_EYE = 305,
     EQTOKEN_EYE_BASE = 306,
     EQTOKEN_BUFFER = 307,
     EQTOKEN_CLEAR = 308,
     EQTOKEN_DRAW = 309,
     EQTOKEN_ASSEMBLE = 310,
     EQTOKEN_READBACK = 311,
     EQTOKEN_COLOR = 312,
     EQTOKEN_DEPTH = 313,
     EQTOKEN_CYCLOP = 314,
     EQTOKEN_LEFT = 315,
     EQTOKEN_RIGHT = 316,
     EQTOKEN_VIEWPORT = 317,
     EQTOKEN_RANGE = 318,
     EQTOKEN_DISPLAY = 319,
     EQTOKEN_SCREEN = 320,
     EQTOKEN_WALL = 321,
     EQTOKEN_BOTTOM_LEFT = 322,
     EQTOKEN_BOTTOM_RIGHT = 323,
     EQTOKEN_TOP_LEFT = 324,
     EQTOKEN_SYNC = 325,
     EQTOKEN_LATENCY = 326,
     EQTOKEN_SWAPBARRIER = 327,
     EQTOKEN_OUTPUTFRAME = 328,
     EQTOKEN_INPUTFRAME = 329,
     EQTOKEN_STRING = 330,
     EQTOKEN_FLOAT = 331,
     EQTOKEN_INTEGER = 332,
     EQTOKEN_UNSIGNED = 333
   };
#endif
/* Tokens.  */
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_SATTR_HOSTNAME 259
#define EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND 260
#define EQTOKEN_CONNECTION_IATTR_TYPE 261
#define EQTOKEN_CONNECTION_IATTR_TCPIP_PORT 262
#define EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT 263
#define EQTOKEN_CONFIG_FATTR_EYE_BASE 264
#define EQTOKEN_WINDOW_IATTR_HINT_STEREO 265
#define EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER 266
#define EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN 267
#define EQTOKEN_WINDOW_IATTR_HINT_DECORATION 268
#define EQTOKEN_WINDOW_IATTR_PLANES_COLOR 269
#define EQTOKEN_WINDOW_IATTR_PLANES_ALPHA 270
#define EQTOKEN_WINDOW_IATTR_PLANES_DEPTH 271
#define EQTOKEN_WINDOW_IATTR_PLANES_STENCIL 272
#define EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS 273
#define EQTOKEN_SERVER 274
#define EQTOKEN_CONFIG 275
#define EQTOKEN_APPNODE 276
#define EQTOKEN_NODE 277
#define EQTOKEN_PIPE 278
#define EQTOKEN_WINDOW 279
#define EQTOKEN_ATTRIBUTES 280
#define EQTOKEN_HINT_STEREO 281
#define EQTOKEN_HINT_DOUBLEBUFFER 282
#define EQTOKEN_HINT_FULLSCREEN 283
#define EQTOKEN_HINT_DECORATION 284
#define EQTOKEN_HINT_STATISTICS 285
#define EQTOKEN_PLANES_COLOR 286
#define EQTOKEN_PLANES_ALPHA 287
#define EQTOKEN_PLANES_DEPTH 288
#define EQTOKEN_PLANES_STENCIL 289
#define EQTOKEN_ON 290
#define EQTOKEN_OFF 291
#define EQTOKEN_AUTO 292
#define EQTOKEN_FASTEST 293
#define EQTOKEN_NICEST 294
#define EQTOKEN_CHANNEL 295
#define EQTOKEN_COMPOUND 296
#define EQTOKEN_CONNECTION 297
#define EQTOKEN_NAME 298
#define EQTOKEN_TYPE 299
#define EQTOKEN_TCPIP 300
#define EQTOKEN_HOSTNAME 301
#define EQTOKEN_COMMAND 302
#define EQTOKEN_TIMEOUT 303
#define EQTOKEN_TASK 304
#define EQTOKEN_EYE 305
#define EQTOKEN_EYE_BASE 306
#define EQTOKEN_BUFFER 307
#define EQTOKEN_CLEAR 308
#define EQTOKEN_DRAW 309
#define EQTOKEN_ASSEMBLE 310
#define EQTOKEN_READBACK 311
#define EQTOKEN_COLOR 312
#define EQTOKEN_DEPTH 313
#define EQTOKEN_CYCLOP 314
#define EQTOKEN_LEFT 315
#define EQTOKEN_RIGHT 316
#define EQTOKEN_VIEWPORT 317
#define EQTOKEN_RANGE 318
#define EQTOKEN_DISPLAY 319
#define EQTOKEN_SCREEN 320
#define EQTOKEN_WALL 321
#define EQTOKEN_BOTTOM_LEFT 322
#define EQTOKEN_BOTTOM_RIGHT 323
#define EQTOKEN_TOP_LEFT 324
#define EQTOKEN_SYNC 325
#define EQTOKEN_LATENCY 326
#define EQTOKEN_SWAPBARRIER 327
#define EQTOKEN_OUTPUTFRAME 328
#define EQTOKEN_INPUTFRAME 329
#define EQTOKEN_STRING 330
#define EQTOKEN_FLOAT 331
#define EQTOKEN_INTEGER 332
#define EQTOKEN_UNSIGNED 333




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
#define YYFINAL  48
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   338

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  83
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  78
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNRULES -- Number of states.  */
#define YYNSTATES  300

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   333

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
       2,    81,     2,    82,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    79,     2,    80,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    38,    41,    44,    47,    50,    53,
      56,    59,    62,    64,    65,    71,    73,    76,    77,    83,
      84,    86,    89,    91,    93,    96,   101,   102,   104,   107,
     110,   112,   115,   117,   119,   120,   126,   127,   133,   134,
     136,   139,   141,   143,   144,   146,   149,   150,   156,   157,
     159,   162,   165,   168,   171,   174,   176,   179,   180,   186,
     187,   189,   192,   194,   197,   200,   203,   205,   208,   209,
     215,   216,   218,   221,   223,   228,   231,   234,   235,   237,
     240,   243,   246,   249,   252,   255,   258,   261,   264,   266,
     269,   270,   276,   277,   279,   282,   285,   290,   293,   294,
     296,   299,   302,   304,   307,   308,   314,   315,   317,   320,
     322,   325,   328,   329,   335,   336,   342,   343,   349,   352,
     358,   360,   362,   364,   366,   367,   369,   372,   374,   376,
     378,   380,   381,   383,   386,   388,   390,   392,   393,   395,
     398,   400,   402,   424,   425,   431,   432,   434,   437,   440,
     441,   447,   448,   454,   455,   457,   460,   463,   466,   467,
     473,   480,   482,   484,   486,   488,   490,   492,   494,   496,
     498,   500,   502
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      84,     0,    -1,    85,    88,    -1,     3,    79,    86,    80,
      -1,    -1,    85,    -1,    86,    85,    -1,     4,   157,    -1,
       5,   157,    -1,     6,    87,    -1,     7,   160,    -1,     8,
     160,    -1,     9,   158,    -1,    10,   156,    -1,    11,   156,
      -1,    12,   156,    -1,    13,   156,    -1,    14,   156,    -1,
      15,   156,    -1,    16,   156,    -1,    17,   156,    -1,    18,
     156,    -1,    45,    -1,    -1,    19,    79,    89,    90,    80,
      -1,    91,    -1,    90,    91,    -1,    -1,    20,    79,    92,
      93,    80,    -1,    -1,    94,    -1,    93,    94,    -1,    97,
      -1,   129,    -1,    71,   160,    -1,    25,    79,    95,    80,
      -1,    -1,    96,    -1,    95,    96,    -1,    51,   158,    -1,
      98,    -1,    97,    98,    -1,   101,    -1,    99,    -1,    -1,
      22,    79,   100,   103,    80,    -1,    -1,    21,    79,   102,
     103,    80,    -1,    -1,   104,    -1,   103,   104,    -1,   105,
      -1,   110,    -1,    -1,   106,    -1,   105,   106,    -1,    -1,
      42,    79,   107,   108,    80,    -1,    -1,   109,    -1,   108,
     109,    -1,    44,    87,    -1,    46,   157,    -1,    47,   157,
      -1,    48,   160,    -1,   111,    -1,   110,   111,    -1,    -1,
      23,    79,   112,   113,    80,    -1,    -1,   114,    -1,   113,
     114,    -1,   115,    -1,    64,   160,    -1,    65,   160,    -1,
      62,   155,    -1,   116,    -1,   115,   116,    -1,    -1,    24,
      79,   117,   118,    80,    -1,    -1,   119,    -1,   118,   119,
      -1,   122,    -1,    25,    79,   120,    80,    -1,    43,   157,
      -1,    62,   155,    -1,    -1,   121,    -1,   120,   121,    -1,
      26,   156,    -1,    27,   156,    -1,    28,   156,    -1,    29,
     156,    -1,    31,   156,    -1,    32,   156,    -1,    33,   156,
      -1,    34,   156,    -1,   123,    -1,   122,   123,    -1,    -1,
      40,    79,   124,   125,    80,    -1,    -1,   126,    -1,   125,
     126,    -1,    43,   157,    -1,    25,    79,   127,    80,    -1,
      62,   155,    -1,    -1,   128,    -1,   127,   128,    -1,    30,
     156,    -1,   130,    -1,   129,   130,    -1,    -1,    41,    79,
     131,   132,    80,    -1,    -1,   133,    -1,   132,   133,    -1,
     130,    -1,    43,   157,    -1,    40,   157,    -1,    -1,    49,
      81,   134,   137,    82,    -1,    -1,    50,    81,   135,   139,
      82,    -1,    -1,    52,    81,   136,   141,    82,    -1,    62,
     155,    -1,    63,    81,   158,   158,    82,    -1,   143,    -1,
     144,    -1,   148,    -1,   150,    -1,    -1,   138,    -1,   137,
     138,    -1,    53,    -1,    54,    -1,    55,    -1,    56,    -1,
      -1,   140,    -1,   139,   140,    -1,    59,    -1,    60,    -1,
      61,    -1,    -1,   142,    -1,   141,   142,    -1,    57,    -1,
      58,    -1,    66,    79,    67,    81,   158,   158,   158,    82,
      68,    81,   158,   158,   158,    82,    69,    81,   158,   158,
     158,    82,    80,    -1,    -1,    72,    79,   145,   146,    80,
      -1,    -1,   147,    -1,   146,   147,    -1,    43,   157,    -1,
      -1,    73,    79,   149,   152,    80,    -1,    -1,    74,    79,
     151,   152,    80,    -1,    -1,   153,    -1,   152,   153,    -1,
      43,   157,    -1,    62,   155,    -1,    -1,    52,    81,   154,
     141,    82,    -1,    81,   158,   158,   158,   158,    82,    -1,
      35,    -1,    36,    -1,    37,    -1,    38,    -1,    39,    -1,
     159,    -1,    75,    -1,    76,    -1,   159,    -1,    77,    -1,
     160,    -1,    78,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   147,   147,   149,   150,   153,   153,   156,   161,   166,
     171,   176,   181,   186,   191,   196,   201,   206,   211,   216,
     221,   226,   232,   234,   234,   237,   237,   238,   238,   241,
     241,   241,   243,   244,   245,   246,   247,   247,   247,   249,
     252,   252,   253,   253,   254,   254,   264,   264,   267,   267,
     267,   269,   270,   271,   272,   272,   274,   273,   280,   280,
     281,   283,   284,   285,   286,   289,   289,   290,   290,   293,
     293,   293,   295,   296,   297,   298,   304,   304,   305,   305,
     308,   308,   308,   310,   311,   312,   313,   321,   321,   321,
     323,   325,   327,   329,   331,   333,   335,   337,   340,   340,
     341,   341,   344,   345,   345,   347,   348,   350,   358,   358,
     358,   360,   364,   364,   366,   365,   377,   377,   378,   380,
     381,   382,   390,   390,   392,   392,   394,   394,   396,   398,
     400,   401,   402,   403,   405,   405,   405,   407,   408,   409,
     410,   412,   412,   412,   414,   415,   416,   418,   418,   418,
     420,   421,   423,   444,   444,   450,   450,   451,   453,   455,
     455,   461,   461,   467,   467,   467,   469,   470,   472,   472,
     475,   484,   485,   486,   487,   488,   489,   491,   498,   499,
     501,   502,   503
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQTOKEN_GLOBAL",
  "EQTOKEN_CONNECTION_SATTR_HOSTNAME",
  "EQTOKEN_CONNECTION_SATTR_LAUNCH_COMMAND",
  "EQTOKEN_CONNECTION_IATTR_TYPE", "EQTOKEN_CONNECTION_IATTR_TCPIP_PORT",
  "EQTOKEN_CONNECTION_IATTR_LAUNCH_TIMEOUT",
  "EQTOKEN_CONFIG_FATTR_EYE_BASE", "EQTOKEN_WINDOW_IATTR_HINT_STEREO",
  "EQTOKEN_WINDOW_IATTR_HINT_DOUBLEBUFFER",
  "EQTOKEN_WINDOW_IATTR_HINT_FULLSCREEN",
  "EQTOKEN_WINDOW_IATTR_HINT_DECORATION",
  "EQTOKEN_WINDOW_IATTR_PLANES_COLOR", "EQTOKEN_WINDOW_IATTR_PLANES_ALPHA",
  "EQTOKEN_WINDOW_IATTR_PLANES_DEPTH",
  "EQTOKEN_WINDOW_IATTR_PLANES_STENCIL",
  "EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS", "EQTOKEN_SERVER",
  "EQTOKEN_CONFIG", "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE",
  "EQTOKEN_WINDOW", "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINT_STEREO",
  "EQTOKEN_HINT_DOUBLEBUFFER", "EQTOKEN_HINT_FULLSCREEN",
  "EQTOKEN_HINT_DECORATION", "EQTOKEN_HINT_STATISTICS",
  "EQTOKEN_PLANES_COLOR", "EQTOKEN_PLANES_ALPHA", "EQTOKEN_PLANES_DEPTH",
  "EQTOKEN_PLANES_STENCIL", "EQTOKEN_ON", "EQTOKEN_OFF", "EQTOKEN_AUTO",
  "EQTOKEN_FASTEST", "EQTOKEN_NICEST", "EQTOKEN_CHANNEL",
  "EQTOKEN_COMPOUND", "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE",
  "EQTOKEN_TCPIP", "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND",
  "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_EYE_BASE",
  "EQTOKEN_BUFFER", "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_ASSEMBLE",
  "EQTOKEN_READBACK", "EQTOKEN_COLOR", "EQTOKEN_DEPTH", "EQTOKEN_CYCLOP",
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
  "channels", "channel", "@8", "channelFields", "channelField",
  "channelAttributes", "channelAttribute", "compounds", "compound", "@9",
  "compoundFields", "compoundField", "@10", "@11", "@12", "compoundTasks",
  "compoundTask", "compoundEyes", "compoundEye", "buffers", "buffer",
  "wall", "swapBarrier", "@13", "swapBarrierFields", "swapBarrierField",
  "outputFrame", "@14", "inputFrame", "@15", "frameFields", "frameField",
  "@16", "viewport", "IATTR", "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
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
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   123,
     125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    83,    84,    85,    85,    86,    86,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    87,    89,    88,    90,    90,    92,    91,    93,
      93,    93,    94,    94,    94,    94,    95,    95,    95,    96,
      97,    97,    98,    98,   100,    99,   102,   101,   103,   103,
     103,   104,   104,   105,   105,   105,   107,   106,   108,   108,
     108,   109,   109,   109,   109,   110,   110,   112,   111,   113,
     113,   113,   114,   114,   114,   114,   115,   115,   117,   116,
     118,   118,   118,   119,   119,   119,   119,   120,   120,   120,
     121,   121,   121,   121,   121,   121,   121,   121,   122,   122,
     124,   123,   125,   125,   125,   126,   126,   126,   127,   127,
     127,   128,   129,   129,   131,   130,   132,   132,   132,   133,
     133,   133,   134,   133,   135,   133,   136,   133,   133,   133,
     133,   133,   133,   133,   137,   137,   137,   138,   138,   138,
     138,   139,   139,   139,   140,   140,   140,   141,   141,   141,
     142,   142,   143,   145,   144,   146,   146,   146,   147,   149,
     148,   151,   150,   152,   152,   152,   153,   153,   154,   153,
     155,   156,   156,   156,   156,   156,   156,   157,   158,   158,
     159,   159,   160
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     0,     5,     1,     2,     0,     5,     0,
       1,     2,     1,     1,     2,     4,     0,     1,     2,     2,
       1,     2,     1,     1,     0,     5,     0,     5,     0,     1,
       2,     1,     1,     0,     1,     2,     0,     5,     0,     1,
       2,     2,     2,     2,     2,     1,     2,     0,     5,     0,
       1,     2,     1,     2,     2,     2,     1,     2,     0,     5,
       0,     1,     2,     1,     4,     2,     2,     0,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     2,
       0,     5,     0,     1,     2,     2,     4,     2,     0,     1,
       2,     2,     1,     2,     0,     5,     0,     1,     2,     1,
       2,     2,     0,     5,     0,     5,     0,     5,     2,     5,
       1,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     0,     1,     2,
       1,     1,    21,     0,     5,     0,     1,     2,     2,     0,
       5,     0,     5,     0,     1,     2,     2,     2,     0,     5,
       6,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
     177,     7,     8,    22,     9,   182,    10,    11,   178,   180,
      12,   179,   181,   171,   172,   173,   174,   175,    13,   176,
      14,    15,    16,    17,    18,    19,    20,    21,     1,     0,
       2,     5,     0,    23,     3,     6,     0,     0,     0,    25,
      27,    24,    26,    29,     0,     0,     0,     0,     0,     0,
      30,    32,    40,    43,    42,    33,   112,    46,    44,    36,
     114,    34,    28,    31,    41,   113,    48,    48,     0,     0,
      37,   116,     0,     0,     0,    49,    51,    54,    52,    65,
       0,    39,    35,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   119,     0,   117,   130,   131,
     132,   133,    67,    56,    47,    50,    55,    66,    45,   121,
     120,   122,   124,   126,     0,   128,     0,     0,   153,   159,
     161,   115,   118,    69,    58,   134,   141,   147,     0,     0,
       0,   155,   163,   163,     0,     0,     0,     0,     0,    70,
      72,    76,     0,     0,     0,     0,     0,    59,   137,   138,
     139,   140,     0,   135,   144,   145,   146,     0,   142,   150,
     151,     0,   148,     0,     0,     0,     0,     0,   156,     0,
       0,     0,     0,   164,     0,    78,    75,    73,    74,    68,
      71,    77,    61,    62,    63,    64,    57,    60,   123,   136,
     125,   143,   127,   149,     0,   129,     0,   158,   154,   157,
     166,   168,   167,   160,   165,   162,    80,     0,     0,   147,
       0,     0,     0,     0,     0,    81,    83,    98,   170,     0,
       0,    87,   100,    85,    86,    79,    82,    99,     0,   169,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
     102,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      84,    89,     0,     0,     0,     0,   103,     0,   108,   105,
     107,   101,   104,     0,     0,     0,   109,     0,   111,   106,
     110,     0,     0,     0,     0,     0,     0,     0,     0,   152
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    52,    24,    50,    56,    58,    59,    63,
      69,    70,    89,    90,    71,    72,    73,    87,    74,    86,
      94,    95,    96,    97,   144,   166,   167,    98,    99,   143,
     158,   159,   160,   161,   226,   234,   235,   258,   259,   236,
     237,   260,   275,   276,   285,   286,    75,    76,    91,   116,
     117,   145,   146,   147,   172,   173,   177,   178,   181,   182,
     118,   119,   151,   187,   188,   120,   152,   121,   153,   192,
     193,   229,   135,    38,    21,    30,    39,    32
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -154
static const yytype_int16 yypact[] =
{
     287,   -32,   -23,   -23,    33,   -24,   -24,   183,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    56,    77,   287,
    -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,    30,
    -154,  -154,    11,  -154,  -154,  -154,   118,    61,    10,  -154,
    -154,  -154,  -154,    89,    68,    88,    96,   107,   -24,    12,
    -154,   104,  -154,  -154,  -154,    60,  -154,  -154,  -154,    98,
    -154,  -154,  -154,  -154,  -154,  -154,    23,    23,   183,    19,
    -154,   191,   114,   116,     8,  -154,   132,  -154,   148,  -154,
       9,  -154,  -154,  -154,   -23,   -23,    31,   131,   139,   152,
     158,   124,   134,   144,   150,  -154,   142,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,   183,  -154,   183,   175,  -154,  -154,
    -154,  -154,  -154,    69,   180,   182,   120,    99,   183,   183,
     171,   212,    92,    92,   177,   152,   -24,   -24,    20,  -154,
     234,  -154,    33,   -23,   -23,   -24,   122,  -154,  -154,  -154,
    -154,  -154,   143,  -154,  -154,  -154,  -154,   129,  -154,  -154,
    -154,    -9,  -154,   183,   119,   183,   -23,    -4,  -154,   -23,
     181,   152,    70,  -154,    93,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,   183,  -154,   183,  -154,  -154,  -154,
    -154,  -154,  -154,  -154,  -154,  -154,   103,   184,   183,    99,
     188,   189,   -23,   152,    15,  -154,   229,  -154,  -154,   190,
      95,   280,  -154,  -154,  -154,  -154,  -154,  -154,   205,  -154,
      81,    81,    81,    81,    81,    81,    81,    81,    35,  -154,
      62,   193,  -154,  -154,  -154,  -154,  -154,  -154,  -154,  -154,
    -154,  -154,   196,   -23,   152,    17,  -154,   183,   247,  -154,
    -154,  -154,  -154,   183,    81,     6,  -154,   183,  -154,  -154,
    -154,   197,   213,   200,   183,   183,   183,   201,   204,  -154
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -154,  -154,    22,  -154,   123,  -154,  -154,  -154,   228,  -154,
    -154,   241,  -154,   226,  -154,   245,  -154,  -154,  -154,  -154,
     230,     4,  -154,   222,  -154,  -154,   153,  -154,   223,  -154,
    -154,   162,  -154,   163,  -154,  -154,    90,  -154,    64,  -154,
      91,  -154,  -154,    50,  -154,    41,  -154,    32,  -154,  -154,
     214,  -154,  -154,  -154,  -154,   156,  -154,   154,   100,  -146,
    -154,  -154,  -154,  -154,   145,  -154,  -154,  -154,  -154,   185,
    -149,  -154,  -153,    -6,    -2,   -77,    -7,     7
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      31,    22,   196,    40,    41,    42,    43,    44,    45,    46,
      47,   101,    26,    27,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      57,    92,    92,    64,    65,   213,   284,    66,   222,   186,
     230,    51,   272,   224,   154,   224,    92,    19,   179,   180,
      93,    93,    20,    67,    25,   231,    48,   148,   232,   149,
     273,   250,   251,   252,   253,    93,   254,   255,   256,   257,
      88,   183,   184,   212,    55,    81,   218,   233,    23,   274,
     244,    31,   155,    68,   156,   157,   289,   272,   124,   128,
      61,    54,    82,   154,   213,   245,    49,   281,   125,   102,
     199,    67,   129,   130,   125,   273,   214,    85,   216,    53,
      64,    65,   131,   189,    66,   270,    33,    34,    35,    36,
      37,   280,   190,   115,   274,    64,    65,    31,   230,    31,
      67,   155,   191,   156,   157,   189,   189,   227,    57,   228,
      60,    31,    31,   231,   190,   190,   232,    77,   115,    88,
     223,   239,   179,   180,   191,   191,   179,   180,    29,    25,
      68,   203,   204,   197,   198,   233,   162,    78,   163,   164,
     165,    92,   205,   225,    93,    79,    31,   249,    31,   174,
     175,   176,   104,    67,   217,   105,    80,   220,   174,   175,
     176,   106,   107,   122,   108,   123,   168,   169,   170,   171,
     283,   215,   206,   137,   109,   110,   287,    31,   111,    31,
     291,   210,   132,   138,   112,   113,   114,   295,   296,   297,
     133,    31,   141,   139,   162,   208,   163,   164,   165,   140,
     243,   104,    67,   134,   105,   168,   169,   170,   171,   136,
     106,   107,   150,   108,   262,   263,   264,   265,   266,   267,
     268,   269,   185,   109,   110,   186,   195,   111,   154,    28,
      29,    25,   221,   112,   113,   114,   238,   241,   242,   231,
      31,   279,   248,   261,   277,   278,    31,   284,   288,   292,
      31,   294,   293,   298,   299,   202,    62,    31,    31,    31,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,   250,   251,   252,   253,
      83,   254,   255,   256,   257,   103,    84,   100,   126,   207,
     200,   127,   271,   201,   246,   282,   290,   247,   209,   240,
     142,   211,   219,     0,     0,     0,     0,     0,   194
};

static const yytype_int16 yycheck[] =
{
       7,     3,   155,     9,    10,    11,    12,    13,    14,    15,
      16,    88,     5,     6,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      20,    23,    23,    21,    22,   181,    30,    25,   191,    43,
      25,    19,    25,   192,    24,   194,    23,    79,    57,    58,
      42,    42,    75,    41,    78,    40,     0,   134,    43,   136,
      43,    26,    27,    28,    29,    42,    31,    32,    33,    34,
      51,   148,   149,    82,    52,    68,    80,    62,    45,    62,
     233,    88,    62,    71,    64,    65,    80,    25,    80,    80,
      80,    80,    80,    24,   240,    80,    19,    80,    94,    80,
      80,    41,   104,   105,   100,    43,   183,    75,   185,    79,
      21,    22,    81,    43,    25,    80,    35,    36,    37,    38,
      39,   274,    52,    91,    62,    21,    22,   134,    25,   136,
      41,    62,    62,    64,    65,    43,    43,   214,    20,   216,
      79,   148,   149,    40,    52,    52,    43,    79,   116,    51,
      80,   228,    57,    58,    62,    62,    57,    58,    77,    78,
      71,   163,   164,   156,   157,    62,    44,    79,    46,    47,
      48,    23,   165,    80,    42,    79,   183,    82,   185,    59,
      60,    61,    40,    41,   186,    43,    79,   189,    59,    60,
      61,    49,    50,    79,    52,    79,    53,    54,    55,    56,
     277,    82,    80,    79,    62,    63,   283,   214,    66,   216,
     287,    82,    81,    79,    72,    73,    74,   294,   295,   296,
      81,   228,    80,    79,    44,    82,    46,    47,    48,    79,
     232,    40,    41,    81,    43,    53,    54,    55,    56,    81,
      49,    50,    67,    52,   250,   251,   252,   253,   254,   255,
     256,   257,    81,    62,    63,    43,    79,    66,    24,    76,
      77,    78,    81,    72,    73,    74,    82,    79,    79,    40,
     277,   273,    82,    68,    81,    79,   283,    30,   284,    82,
     287,    81,    69,    82,    80,   162,    58,   294,   295,   296,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    26,    27,    28,    29,
      69,    31,    32,    33,    34,    89,    71,    87,    96,   166,
     158,    98,   258,   160,   234,   275,   285,   236,   172,   229,
     116,   177,   187,    -1,    -1,    -1,    -1,    -1,   153
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    84,    85,    79,
      75,   157,   157,    45,    87,    78,   160,   160,    76,    77,
     158,   159,   160,    35,    36,    37,    38,    39,   156,   159,
     156,   156,   156,   156,   156,   156,   156,   156,     0,    19,
      88,    85,    86,    79,    80,    85,    89,    20,    90,    91,
      79,    80,    91,    92,    21,    22,    25,    41,    71,    93,
      94,    97,    98,    99,   101,   129,   130,    79,    79,    79,
      79,   160,    80,    94,    98,   130,   102,   100,    51,    95,
      96,   131,    23,    42,   103,   104,   105,   106,   110,   111,
     103,   158,    80,    96,    40,    43,    49,    50,    52,    62,
      63,    66,    72,    73,    74,   130,   132,   133,   143,   144,
     148,   150,    79,    79,    80,   104,   106,   111,    80,   157,
     157,    81,    81,    81,    81,   155,    81,    79,    79,    79,
      79,    80,   133,   112,   107,   134,   135,   136,   158,   158,
      67,   145,   149,   151,    24,    62,    64,    65,   113,   114,
     115,   116,    44,    46,    47,    48,   108,   109,    53,    54,
      55,    56,   137,   138,    59,    60,    61,   139,   140,    57,
      58,   141,   142,   158,   158,    81,    43,   146,   147,    43,
      52,    62,   152,   153,   152,    79,   155,   160,   160,    80,
     114,   116,    87,   157,   157,   160,    80,   109,    82,   138,
      82,   140,    82,   142,   158,    82,   158,   157,    80,   147,
     157,    81,   155,    80,   153,    80,   117,   158,   158,   154,
      25,    40,    43,    62,   118,   119,   122,   123,    82,   158,
     141,    79,    79,   157,   155,    80,   119,   123,    82,    82,
      26,    27,    28,    29,    31,    32,    33,    34,   120,   121,
     124,    68,   156,   156,   156,   156,   156,   156,   156,   156,
      80,   121,    25,    43,    62,   125,   126,    81,    79,   157,
     155,    80,   126,   158,    30,   127,   128,   158,   156,    80,
     128,   158,    82,    69,    81,   158,   158,   158,    82,    80
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
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
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, (yyvsp[(2) - (2)]._string) );
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, (yyvsp[(2) - (2)]._string) );
     ;}
    break;

  case 9:

    { 
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, (yyvsp[(2) - (2)]._connectionType) ); 
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, (yyvsp[(2) - (2)]._unsigned) );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, (yyvsp[(2) - (2)]._unsigned) );
     ;}
    break;

  case 12:

    {
         eqs::Global::instance()->setConfigFAttribute(
             eqs::Config::FATTR_EYE_BASE, (yyvsp[(2) - (2)]._float) );
     ;}
    break;

  case 13:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_STEREO, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 14:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DOUBLEBUFFER, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 15:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_FULLSCREEN, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 16:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_HINT_DECORATION, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 17:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_COLOR, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 18:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 19:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 20:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 21:

    {
         eqs::Global::instance()->setChannelIAttribute(
             eq::Channel::IATTR_HINT_STATISTICS, (yyvsp[(2) - (2)]._int) );
     ;}
    break;

  case 22:

    { (yyval._connectionType) = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 23:

    { server = loader->createServer(); ;}
    break;

  case 27:

    { config = loader->createConfig(); ;}
    break;

  case 28:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 34:

    { config->setLatency( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 39:

    { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, (yyvsp[(2) - (2)]._float) ); ;}
    break;

  case 44:

    { node = loader->createNode(); ;}
    break;

  case 45:

    { 
                        if( node->nConnectionDescriptions() == 0 )
                            node->addConnectionDescription(
                                new eqs::ConnectionDescription( ));

                        config->addNode( node );
                        node = 0; 
                   ;}
    break;

  case 46:

    { node = loader->createNode(); ;}
    break;

  case 47:

    { config->addApplicationNode( node ); node = 0; ;}
    break;

  case 56:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 57:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             ;}
    break;

  case 61:

    { connectionDescription->type = (yyvsp[(2) - (2)]._connectionType); ;}
    break;

  case 62:

    { connectionDescription->hostname = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 63:

    { connectionDescription->launchCommand = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 64:

    { connectionDescription->launchTimeout = (yyvsp[(2) - (2)]._unsigned); ;}
    break;

  case 67:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 68:

    { node->addPipe( eqPipe ); eqPipe = 0; ;}
    break;

  case 73:

    { eqPipe->setDisplay( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 74:

    { eqPipe->setScreen( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 75:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], (int)(yyvsp[(2) - (2)]._viewport)[1],
                                                      (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
        ;}
    break;

  case 78:

    { window = loader->createWindow(); ;}
    break;

  case 79:

    { eqPipe->addWindow( window ); window = 0; ;}
    break;

  case 85:

    { window->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 86:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], 
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3])); 
        ;}
    break;

  case 90:

    { window->setIAttribute( eq::Window::IATTR_HINT_STEREO, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 91:

    { window->setIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 92:

    { window->setIAttribute( eq::Window::IATTR_HINT_FULLSCREEN, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 93:

    { window->setIAttribute( eq::Window::IATTR_HINT_DECORATION, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 94:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 95:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 96:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 97:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 100:

    { channel = loader->createChannel(); ;}
    break;

  case 101:

    { window->addChannel( channel ); channel = 0; ;}
    break;

  case 105:

    { channel->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 107:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0],
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3]));
        ;}
    break;

  case 111:

    { channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 114:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;}
    break;

  case 115:

    { eqCompound = eqCompound->getParent(); ;}
    break;

  case 120:

    { eqCompound->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 121:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[(2) - (2)]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;}
    break;

  case 122:

    { eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 124:

    { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED );;}
    break;

  case 126:

    { flags = eq::Frame::BUFFER_NONE; ;}
    break;

  case 127:

    { eqCompound->setBuffers( flags ); flags = 0; ;}
    break;

  case 128:

    { eqCompound->setViewport( eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3] ));;}
    break;

  case 129:

    { eqCompound->setRange( eq::Range( (yyvsp[(3) - (5)]._float), (yyvsp[(4) - (5)]._float) )); ;}
    break;

  case 137:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 138:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 139:

    { eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);;}
    break;

  case 140:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK);;}
    break;

  case 144:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 145:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 146:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 150:

    { flags |= eq::Frame::BUFFER_COLOR; ;}
    break;

  case 151:

    { flags |= eq::Frame::BUFFER_DEPTH; ;}
    break;

  case 152:

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

  case 153:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 154:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        ;}
    break;

  case 158:

    { swapBarrier->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 159:

    { frame = new eqs::Frame(); ;}
    break;

  case 160:

    { 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        ;}
    break;

  case 161:

    { frame = new eqs::Frame(); ;}
    break;

  case 162:

    { 
            eqCompound->addInputFrame( frame );
            frame = 0;
        ;}
    break;

  case 166:

    { frame->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 167:

    { frame->setViewport(eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3])); ;}
    break;

  case 168:

    { flags = eq::Frame::BUFFER_NONE; ;}
    break;

  case 169:

    { frame->setBuffers( flags ); flags = 0; ;}
    break;

  case 170:

    { 
         (yyval._viewport)[0] = (yyvsp[(2) - (6)]._float);
         (yyval._viewport)[1] = (yyvsp[(3) - (6)]._float);
         (yyval._viewport)[2] = (yyvsp[(4) - (6)]._float);
         (yyval._viewport)[3] = (yyvsp[(5) - (6)]._float);
     ;}
    break;

  case 171:

    { (yyval._int) = eq::ON; ;}
    break;

  case 172:

    { (yyval._int) = eq::OFF; ;}
    break;

  case 173:

    { (yyval._int) = eq::AUTO; ;}
    break;

  case 174:

    { (yyval._int) = eq::FASTEST; ;}
    break;

  case 175:

    { (yyval._int) = eq::NICEST; ;}
    break;

  case 176:

    { (yyval._int) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 177:

    {
         stringBuf = yytext;
         stringBuf.erase( 0, 1 );                  // Leading '"'
         stringBuf.erase( stringBuf.size()-1, 1 ); // Trailing '"'
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 178:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 179:

    { (yyval._float) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 180:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 181:

    { (yyval._int) = (yyvsp[(1) - (1)]._unsigned); ;}
    break;

  case 182:

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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
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

