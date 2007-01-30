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
     EQTOKEN_COMPOUND_IATTR_STEREO_MODE = 273,
     EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK = 274,
     EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK = 275,
     EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS = 276,
     EQTOKEN_SERVER = 277,
     EQTOKEN_CONFIG = 278,
     EQTOKEN_APPNODE = 279,
     EQTOKEN_NODE = 280,
     EQTOKEN_PIPE = 281,
     EQTOKEN_WINDOW = 282,
     EQTOKEN_ATTRIBUTES = 283,
     EQTOKEN_HINT_STEREO = 284,
     EQTOKEN_HINT_DOUBLEBUFFER = 285,
     EQTOKEN_HINT_FULLSCREEN = 286,
     EQTOKEN_HINT_DECORATION = 287,
     EQTOKEN_HINT_STATISTICS = 288,
     EQTOKEN_PLANES_COLOR = 289,
     EQTOKEN_PLANES_ALPHA = 290,
     EQTOKEN_PLANES_DEPTH = 291,
     EQTOKEN_PLANES_STENCIL = 292,
     EQTOKEN_ON = 293,
     EQTOKEN_OFF = 294,
     EQTOKEN_AUTO = 295,
     EQTOKEN_FASTEST = 296,
     EQTOKEN_NICEST = 297,
     EQTOKEN_QUAD = 298,
     EQTOKEN_ANAGLYPH = 299,
     EQTOKEN_RED = 300,
     EQTOKEN_GREEN = 301,
     EQTOKEN_BLUE = 302,
     EQTOKEN_CHANNEL = 303,
     EQTOKEN_COMPOUND = 304,
     EQTOKEN_CONNECTION = 305,
     EQTOKEN_NAME = 306,
     EQTOKEN_TYPE = 307,
     EQTOKEN_TCPIP = 308,
     EQTOKEN_HOSTNAME = 309,
     EQTOKEN_COMMAND = 310,
     EQTOKEN_TIMEOUT = 311,
     EQTOKEN_TASK = 312,
     EQTOKEN_EYE = 313,
     EQTOKEN_EYE_BASE = 314,
     EQTOKEN_BUFFER = 315,
     EQTOKEN_CLEAR = 316,
     EQTOKEN_DRAW = 317,
     EQTOKEN_ASSEMBLE = 318,
     EQTOKEN_READBACK = 319,
     EQTOKEN_COLOR = 320,
     EQTOKEN_DEPTH = 321,
     EQTOKEN_CYCLOP = 322,
     EQTOKEN_LEFT = 323,
     EQTOKEN_RIGHT = 324,
     EQTOKEN_VIEWPORT = 325,
     EQTOKEN_RANGE = 326,
     EQTOKEN_DISPLAY = 327,
     EQTOKEN_SCREEN = 328,
     EQTOKEN_WALL = 329,
     EQTOKEN_BOTTOM_LEFT = 330,
     EQTOKEN_BOTTOM_RIGHT = 331,
     EQTOKEN_TOP_LEFT = 332,
     EQTOKEN_SYNC = 333,
     EQTOKEN_LATENCY = 334,
     EQTOKEN_SWAPBARRIER = 335,
     EQTOKEN_OUTPUTFRAME = 336,
     EQTOKEN_INPUTFRAME = 337,
     EQTOKEN_STEREO_MODE = 338,
     EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK = 339,
     EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK = 340,
     EQTOKEN_STRING = 341,
     EQTOKEN_FLOAT = 342,
     EQTOKEN_INTEGER = 343,
     EQTOKEN_UNSIGNED = 344
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
#define EQTOKEN_COMPOUND_IATTR_STEREO_MODE 273
#define EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK 274
#define EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK 275
#define EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS 276
#define EQTOKEN_SERVER 277
#define EQTOKEN_CONFIG 278
#define EQTOKEN_APPNODE 279
#define EQTOKEN_NODE 280
#define EQTOKEN_PIPE 281
#define EQTOKEN_WINDOW 282
#define EQTOKEN_ATTRIBUTES 283
#define EQTOKEN_HINT_STEREO 284
#define EQTOKEN_HINT_DOUBLEBUFFER 285
#define EQTOKEN_HINT_FULLSCREEN 286
#define EQTOKEN_HINT_DECORATION 287
#define EQTOKEN_HINT_STATISTICS 288
#define EQTOKEN_PLANES_COLOR 289
#define EQTOKEN_PLANES_ALPHA 290
#define EQTOKEN_PLANES_DEPTH 291
#define EQTOKEN_PLANES_STENCIL 292
#define EQTOKEN_ON 293
#define EQTOKEN_OFF 294
#define EQTOKEN_AUTO 295
#define EQTOKEN_FASTEST 296
#define EQTOKEN_NICEST 297
#define EQTOKEN_QUAD 298
#define EQTOKEN_ANAGLYPH 299
#define EQTOKEN_RED 300
#define EQTOKEN_GREEN 301
#define EQTOKEN_BLUE 302
#define EQTOKEN_CHANNEL 303
#define EQTOKEN_COMPOUND 304
#define EQTOKEN_CONNECTION 305
#define EQTOKEN_NAME 306
#define EQTOKEN_TYPE 307
#define EQTOKEN_TCPIP 308
#define EQTOKEN_HOSTNAME 309
#define EQTOKEN_COMMAND 310
#define EQTOKEN_TIMEOUT 311
#define EQTOKEN_TASK 312
#define EQTOKEN_EYE 313
#define EQTOKEN_EYE_BASE 314
#define EQTOKEN_BUFFER 315
#define EQTOKEN_CLEAR 316
#define EQTOKEN_DRAW 317
#define EQTOKEN_ASSEMBLE 318
#define EQTOKEN_READBACK 319
#define EQTOKEN_COLOR 320
#define EQTOKEN_DEPTH 321
#define EQTOKEN_CYCLOP 322
#define EQTOKEN_LEFT 323
#define EQTOKEN_RIGHT 324
#define EQTOKEN_VIEWPORT 325
#define EQTOKEN_RANGE 326
#define EQTOKEN_DISPLAY 327
#define EQTOKEN_SCREEN 328
#define EQTOKEN_WALL 329
#define EQTOKEN_BOTTOM_LEFT 330
#define EQTOKEN_BOTTOM_RIGHT 331
#define EQTOKEN_TOP_LEFT 332
#define EQTOKEN_SYNC 333
#define EQTOKEN_LATENCY 334
#define EQTOKEN_SWAPBARRIER 335
#define EQTOKEN_OUTPUTFRAME 336
#define EQTOKEN_INPUTFRAME 337
#define EQTOKEN_STEREO_MODE 338
#define EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK 339
#define EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK 340
#define EQTOKEN_STRING 341
#define EQTOKEN_FLOAT 342
#define EQTOKEN_INTEGER 343
#define EQTOKEN_UNSIGNED 344




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
    eqNet::ConnectionType   _connectionType;
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
#define YYFINAL  57
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   366

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  94
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  83
/* YYNRULES -- Number of rules.  */
#define YYNRULES  201
/* YYNRULES -- Number of states.  */
#define YYNSTATES  328

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   344

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
       2,    92,     2,    93,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    90,     2,    91,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    38,    41,    44,    47,    50,    53,
      56,    59,    62,    65,    68,    71,    73,    74,    80,    82,
      85,    86,    92,    93,    95,    98,   100,   102,   105,   110,
     111,   113,   116,   119,   121,   124,   126,   128,   129,   135,
     136,   142,   143,   145,   148,   150,   152,   153,   155,   158,
     159,   165,   166,   168,   171,   174,   177,   180,   183,   185,
     188,   189,   195,   196,   198,   201,   203,   206,   209,   212,
     214,   217,   218,   224,   225,   227,   230,   232,   237,   240,
     243,   244,   246,   249,   252,   255,   258,   261,   264,   267,
     270,   273,   275,   278,   279,   285,   286,   288,   291,   294,
     299,   302,   303,   305,   308,   311,   313,   316,   317,   323,
     324,   326,   329,   331,   334,   337,   338,   344,   345,   351,
     352,   358,   361,   367,   369,   371,   373,   375,   380,   381,
     383,   386,   388,   390,   392,   394,   395,   397,   400,   402,
     404,   406,   407,   409,   412,   414,   416,   438,   439,   445,
     446,   448,   451,   454,   455,   461,   462,   468,   469,   471,
     474,   477,   480,   481,   487,   488,   490,   493,   496,   499,
     502,   509,   513,   514,   516,   519,   521,   523,   525,   527,
     529,   531,   533,   535,   537,   539,   541,   543,   545,   547,
     549,   551
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      95,     0,    -1,    96,    99,    -1,     3,    90,    97,    91,
      -1,    -1,    96,    -1,    97,    96,    -1,     4,   173,    -1,
       5,   173,    -1,     6,    98,    -1,     7,   176,    -1,     8,
     176,    -1,     9,   174,    -1,    10,   172,    -1,    11,   172,
      -1,    12,   172,    -1,    13,   172,    -1,    14,   172,    -1,
      15,   172,    -1,    16,   172,    -1,    17,   172,    -1,    21,
     172,    -1,    18,   172,    -1,    19,   169,    -1,    20,   169,
      -1,    53,    -1,    -1,    22,    90,   100,   101,    91,    -1,
     102,    -1,   101,   102,    -1,    -1,    23,    90,   103,   104,
      91,    -1,    -1,   105,    -1,   104,   105,    -1,   108,    -1,
     140,    -1,    79,   176,    -1,    28,    90,   106,    91,    -1,
      -1,   107,    -1,   106,   107,    -1,    59,   174,    -1,   109,
      -1,   108,   109,    -1,   112,    -1,   110,    -1,    -1,    25,
      90,   111,   114,    91,    -1,    -1,    24,    90,   113,   114,
      91,    -1,    -1,   115,    -1,   114,   115,    -1,   116,    -1,
     121,    -1,    -1,   117,    -1,   116,   117,    -1,    -1,    50,
      90,   118,   119,    91,    -1,    -1,   120,    -1,   119,   120,
      -1,    52,    98,    -1,    54,   173,    -1,    55,   173,    -1,
      56,   176,    -1,   122,    -1,   121,   122,    -1,    -1,    26,
      90,   123,   124,    91,    -1,    -1,   125,    -1,   124,   125,
      -1,   126,    -1,    72,   176,    -1,    73,   176,    -1,    70,
     168,    -1,   127,    -1,   126,   127,    -1,    -1,    27,    90,
     128,   129,    91,    -1,    -1,   130,    -1,   129,   130,    -1,
     133,    -1,    28,    90,   131,    91,    -1,    51,   173,    -1,
      70,   168,    -1,    -1,   132,    -1,   131,   132,    -1,    29,
     172,    -1,    30,   172,    -1,    31,   172,    -1,    32,   172,
      -1,    34,   172,    -1,    35,   172,    -1,    36,   172,    -1,
      37,   172,    -1,   134,    -1,   133,   134,    -1,    -1,    48,
      90,   135,   136,    91,    -1,    -1,   137,    -1,   136,   137,
      -1,    51,   173,    -1,    28,    90,   138,    91,    -1,    70,
     168,    -1,    -1,   139,    -1,   138,   139,    -1,    33,   172,
      -1,   141,    -1,   140,   141,    -1,    -1,    49,    90,   142,
     143,    91,    -1,    -1,   144,    -1,   143,   144,    -1,   141,
      -1,    51,   173,    -1,    48,   173,    -1,    -1,    57,    92,
     145,   148,    93,    -1,    -1,    58,    92,   146,   150,    93,
      -1,    -1,    60,    92,   147,   152,    93,    -1,    70,   168,
      -1,    71,    92,   174,   174,    93,    -1,   154,    -1,   155,
      -1,   159,    -1,   161,    -1,    28,    90,   166,    91,    -1,
      -1,   149,    -1,   148,   149,    -1,    61,    -1,    62,    -1,
      63,    -1,    64,    -1,    -1,   151,    -1,   150,   151,    -1,
      67,    -1,    68,    -1,    69,    -1,    -1,   153,    -1,   152,
     153,    -1,    65,    -1,    66,    -1,    74,    90,    75,    92,
     174,   174,   174,    93,    76,    92,   174,   174,   174,    93,
      77,    92,   174,   174,   174,    93,    91,    -1,    -1,    80,
      90,   156,   157,    91,    -1,    -1,   158,    -1,   157,   158,
      -1,    51,   173,    -1,    -1,    81,    90,   160,   163,    91,
      -1,    -1,    82,    90,   162,   163,    91,    -1,    -1,   164,
      -1,   163,   164,    -1,    51,   173,    -1,    70,   168,    -1,
      -1,    60,    92,   165,   152,    93,    -1,    -1,   167,    -1,
     166,   167,    -1,    83,   172,    -1,    84,   169,    -1,    85,
     169,    -1,    92,   174,   174,   174,   174,    93,    -1,    92,
     170,    93,    -1,    -1,   171,    -1,   170,   171,    -1,    45,
      -1,    46,    -1,    47,    -1,    38,    -1,    39,    -1,    40,
      -1,    41,    -1,    42,    -1,    43,    -1,    44,    -1,   175,
      -1,    86,    -1,    87,    -1,   175,    -1,    88,    -1,   176,
      -1,    89,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   158,   158,   160,   161,   164,   164,   167,   172,   177,
     182,   187,   192,   197,   202,   207,   212,   217,   222,   227,
     232,   237,   242,   247,   252,   258,   260,   260,   263,   263,
     264,   264,   267,   267,   267,   269,   270,   271,   272,   273,
     273,   273,   275,   278,   278,   279,   279,   280,   280,   290,
     290,   293,   293,   293,   295,   296,   297,   298,   298,   300,
     299,   306,   306,   307,   309,   310,   311,   312,   315,   315,
     316,   316,   319,   319,   319,   321,   322,   323,   324,   330,
     330,   331,   331,   334,   334,   334,   336,   337,   338,   339,
     347,   347,   347,   349,   351,   353,   355,   357,   359,   361,
     363,   366,   366,   367,   367,   370,   371,   371,   373,   374,
     376,   384,   384,   384,   386,   390,   390,   392,   391,   403,
     403,   404,   406,   407,   408,   416,   416,   418,   418,   420,
     420,   422,   424,   426,   427,   428,   429,   430,   432,   432,
     432,   434,   435,   436,   437,   439,   439,   439,   441,   442,
     443,   445,   445,   445,   447,   448,   450,   471,   471,   477,
     477,   478,   480,   482,   482,   488,   488,   494,   494,   494,
     496,   497,   499,   499,   502,   502,   502,   504,   506,   509,
     513,   521,   523,   524,   525,   527,   528,   529,   532,   533,
     534,   535,   536,   537,   538,   539,   541,   548,   549,   551,
     552,   553
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
  "EQTOKEN_COMPOUND_IATTR_STEREO_MODE",
  "EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_LEFT_MASK",
  "EQTOKEN_COMPOUND_IATTR_STEREO_ANAGLYPH_RIGHT_MASK",
  "EQTOKEN_CHANNEL_IATTR_HINT_STATISTICS", "EQTOKEN_SERVER",
  "EQTOKEN_CONFIG", "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE",
  "EQTOKEN_WINDOW", "EQTOKEN_ATTRIBUTES", "EQTOKEN_HINT_STEREO",
  "EQTOKEN_HINT_DOUBLEBUFFER", "EQTOKEN_HINT_FULLSCREEN",
  "EQTOKEN_HINT_DECORATION", "EQTOKEN_HINT_STATISTICS",
  "EQTOKEN_PLANES_COLOR", "EQTOKEN_PLANES_ALPHA", "EQTOKEN_PLANES_DEPTH",
  "EQTOKEN_PLANES_STENCIL", "EQTOKEN_ON", "EQTOKEN_OFF", "EQTOKEN_AUTO",
  "EQTOKEN_FASTEST", "EQTOKEN_NICEST", "EQTOKEN_QUAD", "EQTOKEN_ANAGLYPH",
  "EQTOKEN_RED", "EQTOKEN_GREEN", "EQTOKEN_BLUE", "EQTOKEN_CHANNEL",
  "EQTOKEN_COMPOUND", "EQTOKEN_CONNECTION", "EQTOKEN_NAME", "EQTOKEN_TYPE",
  "EQTOKEN_TCPIP", "EQTOKEN_HOSTNAME", "EQTOKEN_COMMAND",
  "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_EYE_BASE",
  "EQTOKEN_BUFFER", "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_ASSEMBLE",
  "EQTOKEN_READBACK", "EQTOKEN_COLOR", "EQTOKEN_DEPTH", "EQTOKEN_CYCLOP",
  "EQTOKEN_LEFT", "EQTOKEN_RIGHT", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE",
  "EQTOKEN_DISPLAY", "EQTOKEN_SCREEN", "EQTOKEN_WALL",
  "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT", "EQTOKEN_TOP_LEFT",
  "EQTOKEN_SYNC", "EQTOKEN_LATENCY", "EQTOKEN_SWAPBARRIER",
  "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME", "EQTOKEN_STEREO_MODE",
  "EQTOKEN_STEREO_ANAGLYPH_LEFT_MASK",
  "EQTOKEN_STEREO_ANAGLYPH_RIGHT_MASK", "EQTOKEN_STRING", "EQTOKEN_FLOAT",
  "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED", "'{'", "'}'", "'['", "']'",
  "$accept", "file", "global", "globals", "connectionType", "server", "@1",
  "configs", "config", "@2", "configFields", "configField",
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
  "@16", "compoundAttributes", "compoundAttribute", "viewport",
  "colorMask", "colorMaskBits", "colorMaskBit", "IATTR", "STRING", "FLOAT",
  "INTEGER", "UNSIGNED", 0
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     123,   125,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    94,    95,    96,    96,    97,    97,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    98,   100,    99,   101,   101,
     103,   102,   104,   104,   104,   105,   105,   105,   105,   106,
     106,   106,   107,   108,   108,   109,   109,   111,   110,   113,
     112,   114,   114,   114,   115,   115,   116,   116,   116,   118,
     117,   119,   119,   119,   120,   120,   120,   120,   121,   121,
     123,   122,   124,   124,   124,   125,   125,   125,   125,   126,
     126,   128,   127,   129,   129,   129,   130,   130,   130,   130,
     131,   131,   131,   132,   132,   132,   132,   132,   132,   132,
     132,   133,   133,   135,   134,   136,   136,   136,   137,   137,
     137,   138,   138,   138,   139,   140,   140,   142,   141,   143,
     143,   143,   144,   144,   144,   145,   144,   146,   144,   147,
     144,   144,   144,   144,   144,   144,   144,   144,   148,   148,
     148,   149,   149,   149,   149,   150,   150,   150,   151,   151,
     151,   152,   152,   152,   153,   153,   154,   156,   155,   157,
     157,   157,   158,   160,   159,   162,   161,   163,   163,   163,
     164,   164,   165,   164,   166,   166,   166,   167,   167,   167,
     168,   169,   170,   170,   170,   171,   171,   171,   172,   172,
     172,   172,   172,   172,   172,   172,   173,   174,   174,   175,
     175,   176
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     0,     5,     1,     2,
       0,     5,     0,     1,     2,     1,     1,     2,     4,     0,
       1,     2,     2,     1,     2,     1,     1,     0,     5,     0,
       5,     0,     1,     2,     1,     1,     0,     1,     2,     0,
       5,     0,     1,     2,     2,     2,     2,     2,     1,     2,
       0,     5,     0,     1,     2,     1,     2,     2,     2,     1,
       2,     0,     5,     0,     1,     2,     1,     4,     2,     2,
       0,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     2,     0,     5,     0,     1,     2,     2,     4,
       2,     0,     1,     2,     2,     1,     2,     0,     5,     0,
       1,     2,     1,     2,     2,     0,     5,     0,     5,     0,
       5,     2,     5,     1,     1,     1,     1,     4,     0,     1,
       2,     1,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     0,     1,     2,     1,     1,    21,     0,     5,     0,
       1,     2,     2,     0,     5,     0,     5,     0,     1,     2,
       2,     2,     0,     5,     0,     1,     2,     2,     2,     2,
       6,     3,     0,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     4,   196,     7,     8,    25,     9,   201,    10,
      11,   197,   199,    12,   198,   200,   188,   189,   190,   191,
     192,   193,   194,    13,   195,    14,    15,    16,    17,    18,
      19,    20,    22,   182,    23,    24,    21,     1,     0,     2,
       5,     0,   185,   186,   187,     0,   183,    26,     3,     6,
     181,   184,     0,     0,     0,    28,    30,    27,    29,    32,
       0,     0,     0,     0,     0,     0,    33,    35,    43,    46,
      45,    36,   115,    49,    47,    39,   117,    37,    31,    34,
      44,   116,    51,    51,     0,     0,    40,   119,     0,     0,
       0,    52,    54,    57,    55,    68,     0,    42,    38,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   122,     0,   120,   133,   134,   135,   136,    70,
      59,    50,    53,    58,    69,    48,   174,   124,   123,   125,
     127,   129,     0,   131,     0,     0,   157,   163,   165,   118,
     121,    72,    61,     0,     0,     0,     0,   175,   138,   145,
     151,     0,     0,     0,   159,   167,   167,     0,     0,     0,
       0,     0,    73,    75,    79,     0,     0,     0,     0,     0,
      62,   177,   178,   179,   137,   176,   141,   142,   143,   144,
       0,   139,   148,   149,   150,     0,   146,   154,   155,     0,
     152,     0,     0,     0,     0,     0,   160,     0,     0,     0,
       0,   168,     0,    81,    78,    76,    77,    71,    74,    80,
      64,    65,    66,    67,    60,    63,   126,   140,   128,   147,
     130,   153,     0,   132,     0,   162,   158,   161,   170,   172,
     171,   164,   169,   166,    83,     0,     0,   151,     0,     0,
       0,     0,     0,    84,    86,   101,   180,     0,     0,    90,
     103,    88,    89,    82,    85,   102,     0,   173,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    91,   105,     0,
      93,    94,    95,    96,    97,    98,    99,   100,    87,    92,
       0,     0,     0,     0,   106,     0,   111,   108,   110,   104,
     107,     0,     0,     0,   112,     0,   114,   109,   113,     0,
       0,     0,     0,     0,     0,     0,     0,   156
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    20,    21,    61,    27,    59,    72,    74,    75,    79,
      85,    86,   105,   106,    87,    88,    89,   103,    90,   102,
     110,   111,   112,   113,   162,   189,   190,   114,   115,   161,
     181,   182,   183,   184,   254,   262,   263,   286,   287,   264,
     265,   288,   303,   304,   313,   314,    91,    92,   107,   133,
     134,   168,   169,   170,   200,   201,   205,   206,   209,   210,
     135,   136,   174,   215,   216,   137,   175,   138,   176,   220,
     221,   257,   166,   167,   153,    54,    65,    66,    43,    24,
      33,    44,    35
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -193
static const yytype_int16 yypact[] =
{
     315,   -72,   -40,   -40,    25,    37,    37,    43,   204,   204,
     204,   204,   204,   204,   204,   204,   204,    36,    36,   204,
     134,   117,   315,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,   148,  -193,  -193,  -193,  -193,    51,  -193,
    -193,    19,  -193,  -193,  -193,    10,  -193,  -193,  -193,  -193,
    -193,  -193,   145,    79,   -12,  -193,  -193,  -193,  -193,    74,
      83,    84,   108,   115,    37,    17,  -193,    93,  -193,  -193,
    -193,   162,  -193,  -193,  -193,   155,  -193,  -193,  -193,  -193,
    -193,  -193,    75,    75,    43,   -16,  -193,   159,   128,   142,
      21,  -193,   174,  -193,   208,  -193,    22,  -193,  -193,  -193,
     146,   -40,   -40,   136,   158,   165,   169,   175,   170,   201,
     205,   206,  -193,   101,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,  -193,  -193,  -193,   202,  -193,  -193,  -193,
    -193,  -193,    43,  -193,    43,   163,  -193,  -193,  -193,  -193,
    -193,    65,   124,   204,    36,    36,   168,  -193,   220,   221,
      90,    43,    43,   209,   243,   126,   126,   207,   169,    37,
      37,   -11,  -193,   273,  -193,    25,   -40,   -40,    37,   171,
    -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
     138,  -193,  -193,  -193,  -193,   187,  -193,  -193,  -193,    50,
    -193,    43,   210,    43,   -40,    14,  -193,   -40,   213,   169,
       3,  -193,   100,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,    43,  -193,    43,  -193,  -193,  -193,  -193,  -193,
    -193,  -193,  -193,  -193,    76,   214,    43,    90,   212,   219,
     -40,   169,    16,  -193,   262,  -193,  -193,   218,    97,   234,
    -193,  -193,  -193,  -193,  -193,  -193,   236,  -193,   204,   204,
     204,   204,   204,   204,   204,   204,    53,  -193,    63,   222,
    -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,  -193,
     223,   -40,   169,    30,  -193,    43,   304,  -193,  -193,  -193,
    -193,    43,   204,   -14,  -193,    43,  -193,  -193,  -193,   245,
     263,   247,    43,    43,    43,   248,   251,  -193
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -193,  -193,    -1,  -193,   160,  -193,  -193,  -193,   269,  -193,
    -193,   259,  -193,   241,  -193,   260,  -193,  -193,  -193,  -193,
     246,   -42,  -193,   238,  -193,  -193,   164,  -193,   237,  -193,
    -193,   167,  -193,   172,  -193,  -193,    92,  -193,    66,  -193,
      94,  -193,  -193,    54,  -193,    46,  -193,    15,  -193,  -193,
     227,  -193,  -193,  -193,  -193,   156,  -193,   157,   104,  -192,
    -193,  -193,  -193,  -193,   149,  -193,  -193,  -193,  -193,   189,
    -171,  -193,  -193,   197,  -166,     2,  -193,   301,    -6,    -2,
    -102,    -7,     9
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      34,    25,   117,    45,    46,    47,    48,    49,    50,    51,
      52,    73,   224,    56,    29,    30,   177,   241,    22,   312,
      55,    60,     1,     2,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    80,    81,   104,   258,    82,    23,   108,   108,   252,
     171,   252,   172,   250,   217,    62,    63,    64,   300,   178,
      69,   179,   180,   218,   259,   214,    83,   260,   142,   211,
     212,   109,   109,   219,   142,   118,   241,   317,    26,    77,
     227,   301,   278,   279,   280,   281,   261,   282,   283,   284,
     285,   300,   177,    97,   251,   272,    84,    34,    80,    81,
     302,   108,    82,    70,   258,   246,   101,   273,    98,   242,
      68,   244,   141,   145,   301,   207,   208,    80,    81,   147,
     148,   309,   132,    83,   259,   109,    28,   260,    53,   120,
      31,    32,    28,   302,    57,   178,   308,   179,   180,    58,
     255,    67,   256,   240,   298,    34,   261,    34,   132,   121,
      83,   217,   122,    84,   267,   207,   208,   191,   123,   124,
     218,   125,   207,   208,    34,    34,   192,   193,    73,    76,
     219,   126,   127,    93,    94,   128,   185,   217,   186,   187,
     188,   129,   130,   131,   231,   232,   218,   120,   225,   226,
     277,   253,   159,    62,    63,    64,   219,   233,    95,   196,
     197,   198,   199,   311,    34,    96,    34,   121,    83,   315,
     122,    83,   245,   319,   104,   248,   123,   124,   139,   125,
     323,   324,   325,   185,   109,   186,   187,   188,   149,   126,
     127,   236,   140,   128,   108,    34,   146,    34,   173,   129,
     130,   131,    36,    37,    38,    39,    40,    41,    42,    34,
     150,   163,   164,   165,   202,   203,   204,   151,   271,   194,
     155,   152,   234,   278,   279,   280,   281,   154,   282,   283,
     284,   285,   290,   291,   292,   293,   294,   295,   296,   297,
     238,   196,   197,   198,   199,   163,   164,   165,   202,   203,
     204,   156,    32,    28,   214,   157,   158,   223,    34,   307,
     177,   213,   269,   243,    34,   249,   316,   266,    34,   270,
     259,   276,   289,   306,   305,    34,    34,    34,     1,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,   312,   320,   322,
     321,   326,   327,    78,    99,   230,   119,   100,   228,   116,
     143,   144,   299,   235,   274,   229,   237,   310,   275,   318,
     160,   268,   239,   195,   247,   222,    71
};

static const yytype_uint16 yycheck[] =
{
       7,     3,   104,     9,    10,    11,    12,    13,    14,    15,
      16,    23,   178,    19,     5,     6,    27,   209,    90,    33,
      18,    22,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    24,    25,    59,    28,    28,    86,    26,    26,   220,
     152,   222,   154,   219,    51,    45,    46,    47,    28,    70,
      61,    72,    73,    60,    48,    51,    49,    51,   110,   171,
     172,    50,    50,    70,   116,    91,   268,    91,    53,    91,
      91,    51,    29,    30,    31,    32,    70,    34,    35,    36,
      37,    28,    27,    84,    91,   261,    79,   104,    24,    25,
      70,    26,    28,    93,    28,    91,    91,    91,    91,   211,
      91,   213,    91,    91,    51,    65,    66,    24,    25,   121,
     122,    91,   107,    49,    48,    50,    89,    51,    92,    28,
      87,    88,    89,    70,     0,    70,   302,    72,    73,    22,
     242,    90,   244,    93,    91,   152,    70,   154,   133,    48,
      49,    51,    51,    79,   256,    65,    66,   163,    57,    58,
      60,    60,    65,    66,   171,   172,   164,   165,    23,    90,
      70,    70,    71,    90,    90,    74,    52,    51,    54,    55,
      56,    80,    81,    82,   186,   187,    60,    28,   179,   180,
      93,    91,    91,    45,    46,    47,    70,   188,    90,    61,
      62,    63,    64,   305,   211,    90,   213,    48,    49,   311,
      51,    49,   214,   315,    59,   217,    57,    58,    90,    60,
     322,   323,   324,    52,    50,    54,    55,    56,    92,    70,
      71,    93,    90,    74,    26,   242,    90,   244,    75,    80,
      81,    82,    38,    39,    40,    41,    42,    43,    44,   256,
      92,    83,    84,    85,    67,    68,    69,    92,   260,    91,
      90,    92,    91,    29,    30,    31,    32,    92,    34,    35,
      36,    37,   278,   279,   280,   281,   282,   283,   284,   285,
      93,    61,    62,    63,    64,    83,    84,    85,    67,    68,
      69,    90,    88,    89,    51,    90,    90,    90,   305,   301,
      27,    92,    90,    93,   311,    92,   312,    93,   315,    90,
      48,    93,    76,    90,    92,   322,   323,   324,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    33,    93,    92,
      77,    93,    91,    74,    85,   185,   105,    87,   181,   103,
     112,   114,   286,   189,   262,   183,   200,   303,   264,   313,
     133,   257,   205,   166,   215,   176,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      95,    96,    90,    86,   173,   173,    53,    98,    89,   176,
     176,    87,    88,   174,   175,   176,    38,    39,    40,    41,
      42,    43,    44,   172,   175,   172,   172,   172,   172,   172,
     172,   172,   172,    92,   169,   169,   172,     0,    22,    99,
      96,    97,    45,    46,    47,   170,   171,    90,    91,    96,
      93,   171,   100,    23,   101,   102,    90,    91,   102,   103,
      24,    25,    28,    49,    79,   104,   105,   108,   109,   110,
     112,   140,   141,    90,    90,    90,    90,   176,    91,   105,
     109,   141,   113,   111,    59,   106,   107,   142,    26,    50,
     114,   115,   116,   117,   121,   122,   114,   174,    91,   107,
      28,    48,    51,    57,    58,    60,    70,    71,    74,    80,
      81,    82,   141,   143,   144,   154,   155,   159,   161,    90,
      90,    91,   115,   117,   122,    91,    90,   173,   173,    92,
      92,    92,    92,   168,    92,    90,    90,    90,    90,    91,
     144,   123,   118,    83,    84,    85,   166,   167,   145,   146,
     147,   174,   174,    75,   156,   160,   162,    27,    70,    72,
      73,   124,   125,   126,   127,    52,    54,    55,    56,   119,
     120,   172,   169,   169,    91,   167,    61,    62,    63,    64,
     148,   149,    67,    68,    69,   150,   151,    65,    66,   152,
     153,   174,   174,    92,    51,   157,   158,    51,    60,    70,
     163,   164,   163,    90,   168,   176,   176,    91,   125,   127,
      98,   173,   173,   176,    91,   120,    93,   149,    93,   151,
      93,   153,   174,    93,   174,   173,    91,   158,   173,    92,
     168,    91,   164,    91,   128,   174,   174,   165,    28,    48,
      51,    70,   129,   130,   133,   134,    93,   174,   152,    90,
      90,   173,   168,    91,   130,   134,    93,    93,    29,    30,
      31,    32,    34,    35,    36,    37,   131,   132,   135,    76,
     172,   172,   172,   172,   172,   172,   172,   172,    91,   132,
      28,    51,    70,   136,   137,    92,    90,   173,   168,    91,
     137,   174,    33,   138,   139,   174,   172,    91,   139,   174,
      93,    77,    92,   174,   174,   174,    93,    91
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

    { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_MODE, (yyvsp[(2) - (2)]._int) ); 
     ;}
    break;

  case 23:

    { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, (yyvsp[(2) - (2)]._unsigned) ); 
     ;}
    break;

  case 24:

    { 
         eqs::Global::instance()->setCompoundIAttribute( 
             eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, (yyvsp[(2) - (2)]._unsigned) ); 
     ;}
    break;

  case 25:

    { (yyval._connectionType) = eqNet::CONNECTIONTYPE_TCPIP; ;}
    break;

  case 26:

    { server = loader->createServer(); ;}
    break;

  case 30:

    { config = loader->createConfig(); ;}
    break;

  case 31:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 37:

    { config->setLatency( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 42:

    { config->setFAttribute( 
                             eqs::Config::FATTR_EYE_BASE, (yyvsp[(2) - (2)]._float) ); ;}
    break;

  case 47:

    { node = loader->createNode(); ;}
    break;

  case 48:

    { 
                        if( node->nConnectionDescriptions() == 0 )
                            node->addConnectionDescription(
                                new eqs::ConnectionDescription( ));

                        config->addNode( node );
                        node = 0; 
                   ;}
    break;

  case 49:

    { node = loader->createNode(); ;}
    break;

  case 50:

    { config->addApplicationNode( node ); node = 0; ;}
    break;

  case 59:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 60:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = 0;
             ;}
    break;

  case 64:

    { connectionDescription->type = (yyvsp[(2) - (2)]._connectionType); ;}
    break;

  case 65:

    { connectionDescription->hostname = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 66:

    { connectionDescription->launchCommand = (yyvsp[(2) - (2)]._string); ;}
    break;

  case 67:

    { connectionDescription->launchTimeout = (yyvsp[(2) - (2)]._unsigned); ;}
    break;

  case 70:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 71:

    { node->addPipe( eqPipe ); eqPipe = 0; ;}
    break;

  case 76:

    { eqPipe->setDisplay( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 77:

    { eqPipe->setScreen( (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 78:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], (int)(yyvsp[(2) - (2)]._viewport)[1],
                                                      (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
        ;}
    break;

  case 81:

    { window = loader->createWindow(); ;}
    break;

  case 82:

    { eqPipe->addWindow( window ); window = 0; ;}
    break;

  case 88:

    { window->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 89:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0], 
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3])); 
        ;}
    break;

  case 93:

    { window->setIAttribute( eq::Window::IATTR_HINT_STEREO, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 94:

    { window->setIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 95:

    { window->setIAttribute( eq::Window::IATTR_HINT_FULLSCREEN, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 96:

    { window->setIAttribute( eq::Window::IATTR_HINT_DECORATION, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 97:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 98:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 99:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 100:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 103:

    { channel = loader->createChannel(); ;}
    break;

  case 104:

    { window->addChannel( channel ); channel = 0; ;}
    break;

  case 108:

    { channel->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 110:

    {
            if( (yyvsp[(2) - (2)]._viewport)[2] > 1 || (yyvsp[(2) - (2)]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[(2) - (2)]._viewport)[0],
                                          (int)(yyvsp[(2) - (2)]._viewport)[1], (int)(yyvsp[(2) - (2)]._viewport)[2], (int)(yyvsp[(2) - (2)]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3]));
        ;}
    break;

  case 114:

    { channel->setIAttribute( eq::Channel::IATTR_HINT_STATISTICS, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 117:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;}
    break;

  case 118:

    { eqCompound = eqCompound->getParent(); ;}
    break;

  case 123:

    { eqCompound->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 124:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[(2) - (2)]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;}
    break;

  case 125:

    { eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 127:

    { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED );;}
    break;

  case 129:

    { flags = eq::Frame::BUFFER_NONE; ;}
    break;

  case 130:

    { eqCompound->setBuffers( flags ); flags = 0; ;}
    break;

  case 131:

    { eqCompound->setViewport( eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3] ));;}
    break;

  case 132:

    { eqCompound->setRange( eq::Range( (yyvsp[(3) - (5)]._float), (yyvsp[(4) - (5)]._float) )); ;}
    break;

  case 141:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 142:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 143:

    { eqCompound->enableTask( eqs::Compound::TASK_ASSEMBLE);;}
    break;

  case 144:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK);;}
    break;

  case 148:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 149:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 150:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 154:

    { flags |= eq::Frame::BUFFER_COLOR; ;}
    break;

  case 155:

    { flags |= eq::Frame::BUFFER_DEPTH; ;}
    break;

  case 156:

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

  case 157:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 158:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = 0;
        ;}
    break;

  case 162:

    { swapBarrier->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 163:

    { frame = new eqs::Frame(); ;}
    break;

  case 164:

    { 
            eqCompound->addOutputFrame( frame );
            frame = 0;
        ;}
    break;

  case 165:

    { frame = new eqs::Frame(); ;}
    break;

  case 166:

    { 
            eqCompound->addInputFrame( frame );
            frame = 0;
        ;}
    break;

  case 170:

    { frame->setName( (yyvsp[(2) - (2)]._string) ); ;}
    break;

  case 171:

    { frame->setViewport(eq::Viewport( (yyvsp[(2) - (2)]._viewport)[0], (yyvsp[(2) - (2)]._viewport)[1], (yyvsp[(2) - (2)]._viewport)[2], (yyvsp[(2) - (2)]._viewport)[3])); ;}
    break;

  case 172:

    { flags = eq::Frame::BUFFER_NONE; ;}
    break;

  case 173:

    { frame->setBuffers( flags ); flags = 0; ;}
    break;

  case 177:

    { eqCompound->setIAttribute( eqs::Compound::IATTR_STEREO_MODE, (yyvsp[(2) - (2)]._int) ); ;}
    break;

  case 178:

    { eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_LEFT_MASK, (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 179:

    { eqCompound->setIAttribute( 
                eqs::Compound::IATTR_STEREO_ANAGLYPH_RIGHT_MASK, (yyvsp[(2) - (2)]._unsigned) ); ;}
    break;

  case 180:

    { 
         (yyval._viewport)[0] = (yyvsp[(2) - (6)]._float);
         (yyval._viewport)[1] = (yyvsp[(3) - (6)]._float);
         (yyval._viewport)[2] = (yyvsp[(4) - (6)]._float);
         (yyval._viewport)[3] = (yyvsp[(5) - (6)]._float);
     ;}
    break;

  case 181:

    { (yyval._unsigned) = (yyvsp[(2) - (3)]._unsigned); ;}
    break;

  case 182:

    { (yyval._unsigned) =eqs::Compound::COLOR_MASK_NONE; ;}
    break;

  case 183:

    { (yyval._unsigned) = (yyvsp[(1) - (1)]._unsigned); ;}
    break;

  case 184:

    { (yyval._unsigned) = ((yyvsp[(1) - (2)]._unsigned) | (yyvsp[(2) - (2)]._unsigned));;}
    break;

  case 185:

    { (yyval._unsigned) = eqs::Compound::COLOR_MASK_RED; ;}
    break;

  case 186:

    { (yyval._unsigned) = eqs::Compound::COLOR_MASK_GREEN; ;}
    break;

  case 187:

    { (yyval._unsigned) = eqs::Compound::COLOR_MASK_BLUE; ;}
    break;

  case 188:

    { (yyval._int) = eq::ON; ;}
    break;

  case 189:

    { (yyval._int) = eq::OFF; ;}
    break;

  case 190:

    { (yyval._int) = eq::AUTO; ;}
    break;

  case 191:

    { (yyval._int) = eq::FASTEST; ;}
    break;

  case 192:

    { (yyval._int) = eq::NICEST; ;}
    break;

  case 193:

    { (yyval._int) = eq::QUAD; ;}
    break;

  case 194:

    { (yyval._int) = eq::ANAGLYPH; ;}
    break;

  case 195:

    { (yyval._int) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 196:

    {
         stringBuf = yytext;
         stringBuf.erase( 0, 1 );                  // Leading '"'
         stringBuf.erase( stringBuf.size()-1, 1 ); // Trailing '"'
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 197:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 198:

    { (yyval._float) = (yyvsp[(1) - (1)]._int); ;}
    break;

  case 199:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 200:

    { (yyval._int) = (yyvsp[(1) - (1)]._unsigned); ;}
    break;

  case 201:

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

