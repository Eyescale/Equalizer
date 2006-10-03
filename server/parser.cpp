/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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

/* Bison version.  */
#define YYBISON_VERSION "2.1"

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
     EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER = 265,
     EQTOKEN_WINDOW_IATTR_PLANES_COLOR = 266,
     EQTOKEN_WINDOW_IATTR_PLANES_ALPHA = 267,
     EQTOKEN_WINDOW_IATTR_PLANES_DEPTH = 268,
     EQTOKEN_WINDOW_IATTR_PLANES_STENCIL = 269,
     EQTOKEN_SERVER = 270,
     EQTOKEN_CONFIG = 271,
     EQTOKEN_APPNODE = 272,
     EQTOKEN_NODE = 273,
     EQTOKEN_PIPE = 274,
     EQTOKEN_WINDOW = 275,
     EQTOKEN_ATTRIBUTES = 276,
     EQTOKEN_HINTS = 277,
     EQTOKEN_STEREO = 278,
     EQTOKEN_DOUBLEBUFFER = 279,
     EQTOKEN_PLANES = 280,
     EQTOKEN_COLOR = 281,
     EQTOKEN_ALPHA = 282,
     EQTOKEN_DEPTH = 283,
     EQTOKEN_STENCIL = 284,
     EQTOKEN_ON = 285,
     EQTOKEN_OFF = 286,
     EQTOKEN_AUTO = 287,
     EQTOKEN_CHANNEL = 288,
     EQTOKEN_COMPOUND = 289,
     EQTOKEN_CONNECTION = 290,
     EQTOKEN_NAME = 291,
     EQTOKEN_TYPE = 292,
     EQTOKEN_TCPIP = 293,
     EQTOKEN_HOSTNAME = 294,
     EQTOKEN_COMMAND = 295,
     EQTOKEN_TIMEOUT = 296,
     EQTOKEN_TASK = 297,
     EQTOKEN_EYE = 298,
     EQTOKEN_FORMAT = 299,
     EQTOKEN_CLEAR = 300,
     EQTOKEN_DRAW = 301,
     EQTOKEN_READBACK = 302,
     EQTOKEN_CYCLOP = 303,
     EQTOKEN_LEFT = 304,
     EQTOKEN_RIGHT = 305,
     EQTOKEN_VIEWPORT = 306,
     EQTOKEN_RANGE = 307,
     EQTOKEN_DISPLAY = 308,
     EQTOKEN_WALL = 309,
     EQTOKEN_BOTTOM_LEFT = 310,
     EQTOKEN_BOTTOM_RIGHT = 311,
     EQTOKEN_TOP_LEFT = 312,
     EQTOKEN_SYNC = 313,
     EQTOKEN_LATENCY = 314,
     EQTOKEN_SWAPBARRIER = 315,
     EQTOKEN_OUTPUTFRAME = 316,
     EQTOKEN_INPUTFRAME = 317,
     EQTOKEN_STRING = 318,
     EQTOKEN_FLOAT = 319,
     EQTOKEN_INTEGER = 320,
     EQTOKEN_UNSIGNED = 321
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
#define EQTOKEN_WINDOW_IATTR_HINTS_DOUBLEBUFFER 265
#define EQTOKEN_WINDOW_IATTR_PLANES_COLOR 266
#define EQTOKEN_WINDOW_IATTR_PLANES_ALPHA 267
#define EQTOKEN_WINDOW_IATTR_PLANES_DEPTH 268
#define EQTOKEN_WINDOW_IATTR_PLANES_STENCIL 269
#define EQTOKEN_SERVER 270
#define EQTOKEN_CONFIG 271
#define EQTOKEN_APPNODE 272
#define EQTOKEN_NODE 273
#define EQTOKEN_PIPE 274
#define EQTOKEN_WINDOW 275
#define EQTOKEN_ATTRIBUTES 276
#define EQTOKEN_HINTS 277
#define EQTOKEN_STEREO 278
#define EQTOKEN_DOUBLEBUFFER 279
#define EQTOKEN_PLANES 280
#define EQTOKEN_COLOR 281
#define EQTOKEN_ALPHA 282
#define EQTOKEN_DEPTH 283
#define EQTOKEN_STENCIL 284
#define EQTOKEN_ON 285
#define EQTOKEN_OFF 286
#define EQTOKEN_AUTO 287
#define EQTOKEN_CHANNEL 288
#define EQTOKEN_COMPOUND 289
#define EQTOKEN_CONNECTION 290
#define EQTOKEN_NAME 291
#define EQTOKEN_TYPE 292
#define EQTOKEN_TCPIP 293
#define EQTOKEN_HOSTNAME 294
#define EQTOKEN_COMMAND 295
#define EQTOKEN_TIMEOUT 296
#define EQTOKEN_TASK 297
#define EQTOKEN_EYE 298
#define EQTOKEN_FORMAT 299
#define EQTOKEN_CLEAR 300
#define EQTOKEN_DRAW 301
#define EQTOKEN_READBACK 302
#define EQTOKEN_CYCLOP 303
#define EQTOKEN_LEFT 304
#define EQTOKEN_RIGHT 305
#define EQTOKEN_VIEWPORT 306
#define EQTOKEN_RANGE 307
#define EQTOKEN_DISPLAY 308
#define EQTOKEN_WALL 309
#define EQTOKEN_BOTTOM_LEFT 310
#define EQTOKEN_BOTTOM_RIGHT 311
#define EQTOKEN_TOP_LEFT 312
#define EQTOKEN_SYNC 313
#define EQTOKEN_LATENCY 314
#define EQTOKEN_SWAPBARRIER 315
#define EQTOKEN_OUTPUTFRAME 316
#define EQTOKEN_INPUTFRAME 317
#define EQTOKEN_STRING 318
#define EQTOKEN_FLOAT 319
#define EQTOKEN_INTEGER 320
#define EQTOKEN_UNSIGNED 321




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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
} YYSTYPE;
/* Line 196 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
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
	  YYSIZE_T yyi;				\
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
#define YYFINAL  36
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   292

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  78
/* YYNRULES -- Number of rules. */
#define YYNRULES  163
/* YYNRULES -- Number of states. */
#define YYNSTATES  276

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   321

#define YYTRANSLATE(YYX)						\
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
       2,    69,     2,    70,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    67,     2,    68,     2,     2,     2,     2,
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
      65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    35,    38,    41,    44,    47,    50,    52,
      53,    59,    61,    64,    65,    73,    74,    76,    79,    82,
      84,    87,    89,    91,    92,   100,   101,   109,   110,   112,
     115,   116,   117,   119,   122,   123,   129,   130,   132,   135,
     138,   141,   144,   147,   149,   152,   153,   160,   161,   163,
     166,   169,   172,   174,   177,   178,   185,   186,   188,   191,
     196,   199,   202,   203,   205,   208,   213,   218,   219,   221,
     224,   227,   230,   231,   233,   236,   239,   242,   245,   248,
     250,   253,   254,   260,   261,   263,   266,   269,   272,   274,
     277,   278,   286,   287,   289,   290,   292,   295,   298,   301,
     302,   308,   309,   315,   316,   322,   325,   331,   333,   335,
     337,   339,   340,   342,   345,   347,   349,   351,   352,   354,
     357,   359,   361,   363,   364,   366,   369,   371,   373,   395,
     396,   402,   403,   405,   408,   411,   412,   418,   419,   425,
     426,   428,   431,   434,   441,   443,   445,   447,   449,   451,
     453,   455,   457,   459
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
      72,     0,    -1,    73,    76,    -1,     3,    67,    74,    68,
      -1,    -1,    73,    -1,    74,    73,    -1,     4,    75,    -1,
       5,   145,    -1,     6,   148,    -1,     7,   148,    -1,     8,
     145,    -1,     9,   144,    -1,    10,   144,    -1,    11,   144,
      -1,    12,   144,    -1,    13,   144,    -1,    14,   144,    -1,
      38,    -1,    -1,    15,    67,    77,    78,    68,    -1,    79,
      -1,    78,    79,    -1,    -1,    16,    67,    80,    81,    83,
     117,    68,    -1,    -1,    82,    -1,    81,    82,    -1,    59,
     148,    -1,    84,    -1,    83,    84,    -1,    87,    -1,    85,
      -1,    -1,    18,    67,    86,    91,    89,    96,    68,    -1,
      -1,    17,    67,    88,    91,    89,    96,    68,    -1,    -1,
      90,    -1,    89,    90,    -1,    -1,    -1,    92,    -1,    91,
      92,    -1,    -1,    35,    67,    93,    94,    68,    -1,    -1,
      95,    -1,    94,    95,    -1,    37,    75,    -1,    39,   145,
      -1,    40,   145,    -1,    41,   148,    -1,    97,    -1,    96,
      97,    -1,    -1,    19,    67,    98,    99,   101,    68,    -1,
      -1,   100,    -1,    99,   100,    -1,    53,   148,    -1,    51,
     143,    -1,   102,    -1,   101,   102,    -1,    -1,    20,    67,
     103,   104,   112,    68,    -1,    -1,   105,    -1,   104,   105,
      -1,    21,    67,   106,    68,    -1,    36,   145,    -1,    51,
     143,    -1,    -1,   107,    -1,   106,   107,    -1,    22,    67,
     108,    68,    -1,    25,    67,   110,    68,    -1,    -1,   109,
      -1,   108,   109,    -1,    23,   144,    -1,    24,   144,    -1,
      -1,   111,    -1,   110,   111,    -1,    26,   144,    -1,    27,
     144,    -1,    28,   144,    -1,    29,   144,    -1,   113,    -1,
     112,   113,    -1,    -1,    33,    67,   114,   115,    68,    -1,
      -1,   116,    -1,   115,   116,    -1,    36,   145,    -1,    51,
     143,    -1,   118,    -1,   117,   118,    -1,    -1,    34,    67,
     119,   121,   120,   121,    68,    -1,    -1,   117,    -1,    -1,
     122,    -1,   121,   122,    -1,    36,   145,    -1,    33,   145,
      -1,    -1,    42,    69,   123,   126,    70,    -1,    -1,    43,
      69,   124,   128,    70,    -1,    -1,    44,    69,   125,   130,
      70,    -1,    51,   143,    -1,    52,    69,   146,   146,    70,
      -1,   132,    -1,   133,    -1,   137,    -1,   139,    -1,    -1,
     127,    -1,   126,   127,    -1,    45,    -1,    46,    -1,    47,
      -1,    -1,   129,    -1,   128,   129,    -1,    48,    -1,    49,
      -1,    50,    -1,    -1,   131,    -1,   130,   131,    -1,    26,
      -1,    28,    -1,    54,    67,    55,    69,   146,   146,   146,
      70,    56,    69,   146,   146,   146,    70,    57,    69,   146,
     146,   146,    70,    68,    -1,    -1,    60,    67,   134,   135,
      68,    -1,    -1,   136,    -1,   135,   136,    -1,    36,   145,
      -1,    -1,    61,    67,   138,   141,    68,    -1,    -1,    62,
      67,   140,   141,    68,    -1,    -1,   142,    -1,   141,   142,
      -1,    36,   145,    -1,    69,   146,   146,   146,   146,    70,
      -1,    30,    -1,    31,    -1,    32,    -1,   147,    -1,    63,
      -1,    64,    -1,   147,    -1,    65,    -1,   148,    -1,    66,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   136,   136,   138,   139,   142,   142,   145,   150,   155,
     160,   165,   170,   175,   180,   185,   190,   195,   201,   203,
     203,   206,   206,   207,   207,   210,   210,   210,   212,   214,
     214,   215,   215,   216,   216,   220,   220,   224,   224,   224,
     225,   228,   232,   232,   234,   233,   240,   240,   241,   243,
     244,   245,   246,   249,   249,   250,   250,   253,   253,   253,
     255,   256,   262,   262,   263,   263,   266,   266,   266,   268,
     270,   271,   279,   279,   279,   281,   282,   283,   283,   283,
     285,   287,   289,   289,   289,   291,   293,   295,   297,   300,
     300,   301,   301,   304,   305,   305,   307,   308,   318,   318,
     320,   319,   333,   333,   335,   335,   336,   338,   339,   347,
     347,   349,   349,   351,   351,   353,   355,   357,   358,   359,
     360,   362,   362,   362,   364,   365,   366,   368,   368,   368,
     370,   371,   372,   374,   374,   374,   376,   377,   379,   400,
     400,   406,   406,   407,   409,   411,   411,   417,   417,   423,
     423,   423,   425,   427,   436,   437,   438,   439,   441,   446,
     447,   449,   450,   451
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQTOKEN_GLOBAL",
  "EQTOKEN_CONNECTION_TYPE", "EQTOKEN_CONNECTION_HOSTNAME",
  "EQTOKEN_CONNECTION_TCPIP_PORT", "EQTOKEN_CONNECTION_LAUNCH_TIMEOUT",
  "EQTOKEN_CONNECTION_LAUNCH_COMMAND", "EQTOKEN_WINDOW_IATTR_HINTS_STEREO",
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
  "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_EYE", "EQTOKEN_FORMAT",
  "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_READBACK", "EQTOKEN_CYCLOP",
  "EQTOKEN_LEFT", "EQTOKEN_RIGHT", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE",
  "EQTOKEN_DISPLAY", "EQTOKEN_WALL", "EQTOKEN_BOTTOM_LEFT",
  "EQTOKEN_BOTTOM_RIGHT", "EQTOKEN_TOP_LEFT", "EQTOKEN_SYNC",
  "EQTOKEN_LATENCY", "EQTOKEN_SWAPBARRIER", "EQTOKEN_OUTPUTFRAME",
  "EQTOKEN_INPUTFRAME", "EQTOKEN_STRING", "EQTOKEN_FLOAT",
  "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED", "'{'", "'}'", "'['", "']'",
  "$accept", "file", "global", "globals", "connectionType", "server", "@1",
  "configs", "config", "@2", "configFields", "configField", "nodes",
  "node", "otherNode", "@3", "appNode", "@4", "nodeFields", "nodeField",
  "connections", "connection", "@5", "connectionFields", "connectionField",
  "pipes", "pipe", "@6", "pipeFields", "pipeField", "windows", "window",
  "@7", "windowFields", "windowField", "windowAttributes",
  "windowAttribute", "windowHints", "windowHint", "windowPlanes",
  "windowPlane", "channels", "channel", "@8", "channelFields",
  "channelField", "compounds", "compound", "@9", "compoundChildren",
  "compoundFields", "compoundField", "@10", "@11", "@12", "compoundTasks",
  "compoundTask", "compoundEyes", "compoundEye", "compoundFormats",
  "compoundFormat", "wall", "swapBarrier", "@13", "swapBarrierFields",
  "swapBarrierField", "outputFrame", "@14", "inputFrame", "@15",
  "frameFields", "frameField", "viewport", "IATTR", "STRING", "FLOAT",
  "INTEGER", "UNSIGNED", 0
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
     315,   316,   317,   318,   319,   320,   321,   123,   125,    91,
      93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    71,    72,    73,    73,    74,    74,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    75,    77,
      76,    78,    78,    80,    79,    81,    81,    81,    82,    83,
      83,    84,    84,    86,    85,    88,    87,    89,    89,    89,
      90,    91,    91,    91,    93,    92,    94,    94,    94,    95,
      95,    95,    95,    96,    96,    98,    97,    99,    99,    99,
     100,   100,   101,   101,   103,   102,   104,   104,   104,   105,
     105,   105,   106,   106,   106,   107,   107,   108,   108,   108,
     109,   109,   110,   110,   110,   111,   111,   111,   111,   112,
     112,   114,   113,   115,   115,   115,   116,   116,   117,   117,
     119,   118,   120,   120,   121,   121,   121,   122,   122,   123,
     122,   124,   122,   125,   122,   122,   122,   122,   122,   122,
     122,   126,   126,   126,   127,   127,   127,   128,   128,   128,
     129,   129,   129,   130,   130,   130,   131,   131,   132,   134,
     133,   135,   135,   135,   136,   138,   137,   140,   139,   141,
     141,   141,   142,   143,   144,   144,   144,   144,   145,   146,
     146,   147,   147,   148
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     4,     0,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     0,
       5,     1,     2,     0,     7,     0,     1,     2,     2,     1,
       2,     1,     1,     0,     7,     0,     7,     0,     1,     2,
       0,     0,     1,     2,     0,     5,     0,     1,     2,     2,
       2,     2,     2,     1,     2,     0,     6,     0,     1,     2,
       2,     2,     1,     2,     0,     6,     0,     1,     2,     4,
       2,     2,     0,     1,     2,     4,     4,     0,     1,     2,
       2,     2,     0,     1,     2,     2,     2,     2,     2,     1,
       2,     0,     5,     0,     1,     2,     2,     2,     1,     2,
       0,     7,     0,     1,     0,     1,     2,     2,     2,     0,
       5,     0,     5,     0,     5,     2,     5,     1,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     0,     1,     2,     1,     1,    21,     0,
       5,     0,     1,     2,     2,     0,     5,     0,     5,     0,
       1,     2,     2,     6,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     4,    18,     7,   158,     8,
     163,     9,    10,    11,   154,   155,   156,   161,    12,   157,
     162,    13,    14,    15,    16,    17,     1,     0,     2,     5,
       0,    19,     3,     6,     0,     0,     0,    21,    23,    20,
      22,    25,     0,     0,    26,    28,     0,     0,    27,     0,
      29,    32,    31,    35,    33,     0,    30,     0,    98,    41,
      41,   100,    24,    99,     0,    37,    42,    37,   104,    44,
       0,    38,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   102,   105,   117,   118,   119,
     120,    46,     0,    39,     0,    53,     0,   108,   107,   109,
     111,   113,     0,   115,     0,     0,   139,   145,   147,   103,
     104,   106,     0,     0,     0,     0,     0,    47,    55,    36,
      54,    34,   121,   127,   133,   159,     0,   160,     0,     0,
     141,   149,   149,     0,    49,    50,    51,    52,    45,    48,
      57,   124,   125,   126,     0,   122,   130,   131,   132,     0,
     128,   136,   137,     0,   134,     0,     0,     0,     0,     0,
     142,     0,     0,   150,     0,   101,     0,     0,     0,    58,
     110,   123,   112,   129,   114,   135,     0,   116,     0,   144,
     140,   143,   152,   146,   151,   148,    61,    60,     0,    59,
       0,    62,     0,     0,    64,    56,    63,   153,     0,    66,
       0,     0,     0,     0,     0,    67,     0,    72,    70,    71,
       0,    68,     0,    89,     0,     0,     0,     0,    73,    91,
      65,    90,     0,    77,    82,    69,    74,    93,     0,     0,
       0,     0,    78,     0,     0,     0,     0,     0,    83,     0,
       0,     0,    94,     0,    80,    81,    75,    79,    85,    86,
      87,    88,    76,    84,    96,    97,    92,    95,     0,     0,
       0,     0,     0,     0,     0,   138
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    13,    14,    40,    17,    38,    44,    46,    47,    51,
      53,    54,    59,    60,    61,    70,    62,    69,    80,    81,
      75,    76,   101,   126,   127,   104,   105,   150,   178,   179,
     200,   201,   209,   214,   215,   227,   228,   241,   242,   247,
     248,   222,   223,   237,   251,   252,    67,    68,    78,   120,
      95,    96,   132,   133,   134,   154,   155,   159,   160,   163,
     164,    97,    98,   140,   169,   170,    99,   141,   100,   142,
     172,   173,   113,    28,    19,   136,   137,    30
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -167
static const short int yypact[] =
{
     204,   -43,    68,   -10,    60,    60,   -10,    62,    62,    62,
      62,    62,    62,    61,   119,   204,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,    63,  -167,  -167,
      23,  -167,  -167,  -167,   126,   108,    -4,  -167,  -167,  -167,
    -167,   120,    60,    28,  -167,  -167,   138,   156,  -167,   115,
    -167,  -167,  -167,  -167,  -167,   158,  -167,    15,  -167,   152,
     152,  -167,  -167,  -167,   159,   152,  -167,   152,   134,  -167,
     174,  -167,  -167,   174,   -10,   -10,   160,   161,   162,   164,
     167,   165,   170,   175,   179,   112,  -167,  -167,  -167,  -167,
    -167,   143,   185,  -167,    -1,  -167,     0,  -167,  -167,  -167,
    -167,  -167,   133,  -167,   133,   172,  -167,  -167,  -167,   194,
     134,  -167,    68,   -10,   -10,    60,    83,  -167,  -167,  -167,
    -167,  -167,   155,   171,    74,  -167,   133,  -167,   133,   178,
     217,   218,   218,    53,  -167,  -167,  -167,  -167,  -167,  -167,
      59,  -167,  -167,  -167,    90,  -167,  -167,  -167,  -167,    91,
    -167,  -167,  -167,    12,  -167,   133,   186,   133,   -10,    35,
    -167,   -10,    40,  -167,    41,  -167,   164,    60,    21,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,   133,  -167,   133,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,   188,  -167,
       1,  -167,   187,   133,  -167,  -167,  -167,  -167,   189,    80,
     202,   193,   -10,   164,    29,  -167,   192,    26,  -167,  -167,
     195,  -167,   -13,  -167,   133,   196,   197,    17,  -167,  -167,
    -167,  -167,   133,   180,   163,  -167,  -167,   102,   133,    62,
      62,    20,  -167,    62,    62,    62,    62,    31,  -167,   -10,
     164,    30,  -167,   198,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,   208,   200,
     133,   133,   133,   201,   199,  -167
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -167,  -167,   128,  -167,   144,  -167,  -167,  -167,   224,  -167,
    -167,   219,  -167,   214,  -167,  -167,  -167,  -167,   203,    64,
     205,    75,  -167,  -167,   148,   206,    65,  -167,  -167,    98,
    -167,    77,  -167,  -167,    67,  -167,    51,  -167,    38,  -167,
      36,  -167,    66,  -167,  -167,    33,   190,   -56,  -167,  -167,
     166,   -73,  -167,  -167,  -167,  -167,   136,  -167,   123,  -167,
     124,  -167,  -167,  -167,  -167,   122,  -167,  -167,  -167,  -167,
     150,    50,  -166,     5,    -6,  -113,    -5,     4
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
      23,   138,    29,    29,    29,    29,    29,    29,    21,    22,
     196,    73,    45,    31,    32,    33,    34,    35,   102,   102,
     220,   198,   121,   165,    15,   166,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,   161,   225,
     162,   198,   226,   239,   240,    56,    57,   219,   225,    65,
     211,   226,   186,    18,   188,   230,    55,   243,   244,   245,
     246,    36,   220,    73,    49,   212,   249,   129,   131,   205,
     121,   168,   176,   202,   177,   203,   171,   171,   107,   108,
     213,   250,   184,    72,   265,   235,    84,    52,   256,    85,
     208,    42,    24,    25,    26,    86,    87,    88,   266,   262,
     161,   211,   162,   190,    89,    90,    16,    91,   193,   195,
     176,   232,   177,    92,    93,    94,   212,   145,   146,   238,
     122,   175,   123,   124,   125,   253,    20,    27,    20,   147,
      41,   213,    56,    57,    37,   151,   152,   153,   249,   156,
     157,   158,    45,    39,   103,    84,    65,   103,    85,    65,
      82,   148,    82,   250,    86,    87,    88,   271,   272,   273,
     180,   182,   189,    89,    90,   192,    91,    84,    43,   130,
      85,   130,    92,    93,    94,    48,    86,    87,    88,    52,
     122,   197,   123,   124,   125,    89,    90,    74,    91,   243,
     244,   245,   246,   102,    92,    93,    94,   135,    27,    20,
     151,   152,   153,   239,   240,    63,   218,     1,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,   156,
     157,   158,   194,    64,   194,    71,    79,   139,    65,   109,
     110,   111,   115,   112,    29,    29,   114,   116,    29,    29,
      29,    29,   117,   264,   254,   255,   118,   167,   258,   259,
     260,   261,   128,   168,   171,   204,   187,   207,   216,   210,
     217,   224,   229,   233,   234,   269,   144,   275,   268,   270,
      50,   274,    58,    66,   149,    77,   199,   206,   236,   257,
      83,   221,   183,   263,   267,   119,   143,   185,   231,   106,
     181,   191,   174
};

static const unsigned short int yycheck[] =
{
       6,   114,     7,     8,     9,    10,    11,    12,     4,     5,
     176,    67,    16,     8,     9,    10,    11,    12,    19,    19,
      33,    20,    95,   136,    67,   138,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    26,    22,
      28,    20,    25,    23,    24,    17,    18,   213,    22,    34,
      21,    25,   165,    63,   167,    68,    52,    26,    27,    28,
      29,     0,    33,   119,    68,    36,    36,    68,    68,    68,
     143,    36,    51,   186,    53,   188,    36,    36,    84,    85,
      51,    51,    70,    68,   250,    68,    33,    59,    68,    36,
     203,    68,    30,    31,    32,    42,    43,    44,    68,    68,
      26,    21,    28,    68,    51,    52,    38,    54,    68,    68,
      51,   224,    53,    60,    61,    62,    36,   123,   124,   232,
      37,    68,    39,    40,    41,   238,    66,    65,    66,   125,
      67,    51,    17,    18,    15,    45,    46,    47,    36,    48,
      49,    50,    16,    15,    80,    33,    34,    83,    36,    34,
      75,    68,    77,    51,    42,    43,    44,   270,   271,   272,
      70,    70,   168,    51,    52,   171,    54,    33,    40,   104,
      36,   106,    60,    61,    62,    67,    42,    43,    44,    59,
      37,   177,    39,    40,    41,    51,    52,    35,    54,    26,
      27,    28,    29,    19,    60,    61,    62,    64,    65,    66,
      45,    46,    47,    23,    24,    67,   212,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    48,
      49,    50,   172,    67,   174,    67,    67,    55,    34,    69,
      69,    69,    67,    69,   239,   240,    69,    67,   243,   244,
     245,   246,    67,   249,   239,   240,    67,    69,   243,   244,
     245,   246,    67,    36,    36,    67,    70,    70,    56,    70,
      67,    69,    67,    67,    67,    57,   122,    68,    70,    69,
      46,    70,    53,    59,   126,    70,   178,   200,   227,   241,
      77,   214,   159,   247,   251,    95,   120,   163,   222,    83,
     154,   169,   142
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    72,    73,    67,    38,    75,    63,   145,
      66,   148,   148,   145,    30,    31,    32,    65,   144,   147,
     148,   144,   144,   144,   144,   144,     0,    15,    76,    73,
      74,    67,    68,    73,    77,    16,    78,    79,    67,    68,
      79,    80,    59,    81,    82,   148,    17,    18,    82,    83,
      84,    85,    87,    67,    67,    34,    84,   117,   118,    88,
      86,    67,    68,   118,    35,    91,    92,    91,   119,    67,
      89,    90,    92,    89,    33,    36,    42,    43,    44,    51,
      52,    54,    60,    61,    62,   121,   122,   132,   133,   137,
     139,    93,    19,    90,    96,    97,    96,   145,   145,    69,
      69,    69,    69,   143,    69,    67,    67,    67,    67,   117,
     120,   122,    37,    39,    40,    41,    94,    95,    67,    68,
      97,    68,   123,   124,   125,    64,   146,   147,   146,    55,
     134,   138,   140,   121,    75,   145,   145,   148,    68,    95,
      98,    45,    46,    47,   126,   127,    48,    49,    50,   128,
     129,    26,    28,   130,   131,   146,   146,    69,    36,   135,
     136,    36,   141,   142,   141,    68,    51,    53,    99,   100,
      70,   127,    70,   129,    70,   131,   146,    70,   146,   145,
      68,   136,   145,    68,   142,    68,   143,   148,    20,   100,
     101,   102,   146,   146,    67,    68,   102,    70,   146,   103,
      70,    21,    36,    51,   104,   105,    56,    67,   145,   143,
      33,   105,   112,   113,    69,    22,    25,   106,   107,    67,
      68,   113,   146,    67,    67,    68,   107,   114,   146,    23,
      24,   108,   109,    26,    27,    28,    29,   110,   111,    36,
      51,   115,   116,   146,   144,   144,    68,   109,   144,   144,
     144,   144,    68,   111,   145,   143,    68,   116,    70,    57,
      69,   146,   146,   146,    70,    68
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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
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
      yysymprint (stderr,					\
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
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
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
  const char *yys = yystr;

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
      size_t yyn = 0;
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

#endif /* YYERROR_VERBOSE */



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
    ;
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

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



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
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
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
         eqs::Global::instance()->setConnectionIAttribute( 
             eqs::ConnectionDescription::IATTR_TYPE, (yyvsp[0]._connectionType) ); 
     ;}
    break;

  case 8:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_HOSTNAME, (yyvsp[0]._string) );
     ;}
    break;

  case 9:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_TCPIP_PORT, (yyvsp[0]._unsigned) );
     ;}
    break;

  case 10:

    {
         eqs::Global::instance()->setConnectionIAttribute(
             eqs::ConnectionDescription::IATTR_LAUNCH_TIMEOUT, (yyvsp[0]._unsigned) );
     ;}
    break;

  case 11:

    {
         eqs::Global::instance()->setConnectionSAttribute(
             eqs::ConnectionDescription::SATTR_LAUNCH_COMMAND, (yyvsp[0]._string) );
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
             eq::Window::IATTR_PLANES_COLOR, (yyvsp[0]._int) );
     ;}
    break;

  case 15:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_ALPHA, (yyvsp[0]._int) );
     ;}
    break;

  case 16:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_DEPTH, (yyvsp[0]._int) );
     ;}
    break;

  case 17:

    {
         eqs::Global::instance()->setWindowIAttribute(
             eq::Window::IATTR_PLANES_STENCIL, (yyvsp[0]._int) );
     ;}
    break;

  case 18:

    { (yyval._connectionType) = eqNet::Connection::TYPE_TCPIP; ;}
    break;

  case 19:

    { server = loader->createServer(); ;}
    break;

  case 23:

    { config = loader->createConfig(); ;}
    break;

  case 24:

    { server->addConfig( config ); config = NULL; ;}
    break;

  case 28:

    { config->setLatency( (yyvsp[0]._unsigned) ); ;}
    break;

  case 33:

    { node = loader->createNode(); ;}
    break;

  case 34:

    { config->addNode( node ); node = NULL; ;}
    break;

  case 35:

    { node = loader->createNode(); ;}
    break;

  case 36:

    { config->addApplicationNode( node ); node = NULL; ;}
    break;

  case 41:

    { // No connection specified, create default from globals
                 node->addConnectionDescription(
                     new eqs::ConnectionDescription( ));
             ;}
    break;

  case 44:

    { connectionDescription = new eqs::ConnectionDescription(); ;}
    break;

  case 45:

    { 
                 node->addConnectionDescription( connectionDescription );
                 connectionDescription = NULL;
             ;}
    break;

  case 49:

    { connectionDescription->type = (yyvsp[0]._connectionType); ;}
    break;

  case 50:

    { connectionDescription->hostname = (yyvsp[0]._string); ;}
    break;

  case 51:

    { connectionDescription->launchCommand = (yyvsp[0]._string); ;}
    break;

  case 52:

    { connectionDescription->launchTimeout = (yyvsp[0]._unsigned); ;}
    break;

  case 55:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 56:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 60:

    { eqPipe->setDisplay( (yyvsp[0]._unsigned) ); ;}
    break;

  case 61:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], (int)(yyvsp[0]._viewport)[1],
                                                      (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
        ;}
    break;

  case 64:

    { window = loader->createWindow(); ;}
    break;

  case 65:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 70:

    { window->setName( (yyvsp[0]._string) ); ;}
    break;

  case 71:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], 
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3])); 
        ;}
    break;

  case 80:

    { window->setIAttribute( eq::Window::IATTR_HINTS_STEREO, (yyvsp[0]._int) ); ;}
    break;

  case 81:

    { window->setIAttribute( eq::Window::IATTR_HINTS_DOUBLEBUFFER, (yyvsp[0]._int) ); ;}
    break;

  case 85:

    { window->setIAttribute( eq::Window::IATTR_PLANES_COLOR, (yyvsp[0]._int) ); ;}
    break;

  case 86:

    { window->setIAttribute( eq::Window::IATTR_PLANES_ALPHA, (yyvsp[0]._int) ); ;}
    break;

  case 87:

    { window->setIAttribute( eq::Window::IATTR_PLANES_DEPTH, (yyvsp[0]._int) ); ;}
    break;

  case 88:

    { window->setIAttribute( eq::Window::IATTR_PLANES_STENCIL, (yyvsp[0]._int) ); ;}
    break;

  case 91:

    { channel = loader->createChannel(); ;}
    break;

  case 92:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 96:

    { channel->setName( (yyvsp[0]._string) ); ;}
    break;

  case 97:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0],
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3]));
        ;}
    break;

  case 100:

    {
                  eqs::Compound* child = loader->createCompound();
                  if( eqCompound )
                      eqCompound->addChild( child );
                  else
                      config->addCompound( child );
                  eqCompound = child;
              ;}
    break;

  case 101:

    { eqCompound = eqCompound->getParent(); ;}
    break;

  case 107:

    { eqCompound->setName( (yyvsp[0]._string) ); ;}
    break;

  case 108:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[0]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             eqCompound->setChannel( channel );
    ;}
    break;

  case 109:

    { eqCompound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 111:

    { eqCompound->setEyes( eqs::Compound::EYE_UNDEFINED ); ;}
    break;

  case 113:

    { eqCompound->setFormats( eq::Frame::FORMAT_UNDEFINED );;}
    break;

  case 115:

    { eqCompound->setViewport( eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3] )); ;}
    break;

  case 116:

    { eqCompound->setRange( eq::Range( (yyvsp[-2]._float), (yyvsp[-1]._float) )); ;}
    break;

  case 124:

    { eqCompound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 125:

    { eqCompound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 126:

    { eqCompound->enableTask( eqs::Compound::TASK_READBACK ); ;}
    break;

  case 130:

    { eqCompound->enableEye( eqs::Compound::EYE_CYCLOP ); ;}
    break;

  case 131:

    { eqCompound->enableEye( eqs::Compound::EYE_LEFT ); ;}
    break;

  case 132:

    { eqCompound->enableEye( eqs::Compound::EYE_RIGHT ); ;}
    break;

  case 136:

    { eqCompound->enableFormat( eq::Frame::FORMAT_COLOR ); ;}
    break;

  case 137:

    { eqCompound->enableFormat( eq::Frame::FORMAT_DEPTH ); ;}
    break;

  case 138:

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

  case 139:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 140:

    { 
            eqCompound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 144:

    { swapBarrier->setName( (yyvsp[0]._string) ); ;}
    break;

  case 145:

    { frame = new eqs::Frame(); ;}
    break;

  case 146:

    { 
            eqCompound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 147:

    { frame = new eqs::Frame(); ;}
    break;

  case 148:

    { 
            eqCompound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 152:

    { frame->setName( (yyvsp[0]._string) ); ;}
    break;

  case 153:

    { 
         (yyval._viewport)[0] = (yyvsp[-4]._float);
         (yyval._viewport)[1] = (yyvsp[-3]._float);
         (yyval._viewport)[2] = (yyvsp[-2]._float);
         (yyval._viewport)[3] = (yyvsp[-1]._float);
     ;}
    break;

  case 154:

    { (yyval._int) = eq::ON; ;}
    break;

  case 155:

    { (yyval._int) = eq::OFF; ;}
    break;

  case 156:

    { (yyval._int) = eq::AUTO; ;}
    break;

  case 157:

    { (yyval._int) = (yyvsp[0]._int); ;}
    break;

  case 158:

    {
         stringBuf = yytext;
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 159:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 160:

    { (yyval._float) = (yyvsp[0]._int); ;}
    break;

  case 161:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 162:

    { (yyval._int) = (yyvsp[0]._unsigned); ;}
    break;

  case 163:

    { (yyval._unsigned) = atoi( yytext ); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */


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
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
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
	  int yychecklim = YYLAST - yyn;
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
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
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
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

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
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
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

