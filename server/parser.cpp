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
     EQTOKEN_FORMAT = 280,
     EQTOKEN_CLEAR = 281,
     EQTOKEN_DRAW = 282,
     EQTOKEN_READBACK = 283,
     EQTOKEN_COLOR = 284,
     EQTOKEN_DEPTH = 285,
     EQTOKEN_VIEWPORT = 286,
     EQTOKEN_RANGE = 287,
     EQTOKEN_DISPLAY = 288,
     EQTOKEN_WALL = 289,
     EQTOKEN_BOTTOM_LEFT = 290,
     EQTOKEN_BOTTOM_RIGHT = 291,
     EQTOKEN_TOP_LEFT = 292,
     EQTOKEN_SYNC = 293,
     EQTOKEN_LATENCY = 294,
     EQTOKEN_SWAPBARRIER = 295,
     EQTOKEN_OUTPUTFRAME = 296,
     EQTOKEN_INPUTFRAME = 297,
     EQTOKEN_STRING = 298,
     EQTOKEN_FLOAT = 299,
     EQTOKEN_INTEGER = 300,
     EQTOKEN_UNSIGNED = 301
   };
#endif
/* Tokens.  */
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
#define EQTOKEN_FORMAT 280
#define EQTOKEN_CLEAR 281
#define EQTOKEN_DRAW 282
#define EQTOKEN_READBACK 283
#define EQTOKEN_COLOR 284
#define EQTOKEN_DEPTH 285
#define EQTOKEN_VIEWPORT 286
#define EQTOKEN_RANGE 287
#define EQTOKEN_DISPLAY 288
#define EQTOKEN_WALL 289
#define EQTOKEN_BOTTOM_LEFT 290
#define EQTOKEN_BOTTOM_RIGHT 291
#define EQTOKEN_TOP_LEFT 292
#define EQTOKEN_SYNC 293
#define EQTOKEN_LATENCY 294
#define EQTOKEN_SWAPBARRIER 295
#define EQTOKEN_OUTPUTFRAME 296
#define EQTOKEN_INPUTFRAME 297
#define EQTOKEN_STRING 298
#define EQTOKEN_FLOAT 299
#define EQTOKEN_INTEGER 300
#define EQTOKEN_UNSIGNED 301




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
#define YYFINAL  18
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   229

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  51
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  68
/* YYNRULES -- Number of rules. */
#define YYNRULES  127
/* YYNRULES -- Number of states. */
#define YYNSTATES  220

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   301

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
       2,    49,     2,    50,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,     2,    48,     2,     2,     2,     2,
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
      45,    46
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     6,    11,    12,    14,    17,    20,    23,
      26,    29,    32,    34,    35,    41,    43,    46,    47,    55,
      56,    58,    61,    64,    66,    69,    71,    73,    74,    82,
      83,    91,    92,    94,    97,    98,    99,   101,   104,   105,
     111,   112,   114,   117,   120,   123,   126,   129,   131,   134,
     135,   142,   143,   145,   148,   151,   154,   156,   159,   160,
     167,   168,   170,   173,   176,   179,   181,   184,   185,   191,
     192,   194,   197,   200,   203,   205,   208,   209,   217,   218,
     220,   221,   223,   226,   229,   232,   233,   239,   240,   246,
     249,   255,   257,   259,   261,   263,   264,   266,   269,   271,
     273,   275,   276,   278,   281,   283,   285,   307,   308,   314,
     315,   317,   320,   323,   324,   330,   331,   337,   338,   340,
     343,   346,   353,   355,   357,   359,   361,   363
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      52,     0,    -1,    53,    56,    -1,     3,    47,    54,    48,
      -1,    -1,    53,    -1,    54,    53,    -1,     4,    55,    -1,
       5,   115,    -1,     6,   118,    -1,     7,   118,    -1,     8,
     115,    -1,    20,    -1,    -1,     9,    47,    57,    58,    48,
      -1,    59,    -1,    58,    59,    -1,    -1,    10,    47,    60,
      61,    63,    91,    48,    -1,    -1,    62,    -1,    61,    62,
      -1,    39,   118,    -1,    64,    -1,    63,    64,    -1,    67,
      -1,    65,    -1,    -1,    12,    47,    66,    71,    69,    76,
      48,    -1,    -1,    11,    47,    68,    71,    69,    76,    48,
      -1,    -1,    70,    -1,    69,    70,    -1,    -1,    -1,    72,
      -1,    71,    72,    -1,    -1,    17,    47,    73,    74,    48,
      -1,    -1,    75,    -1,    74,    75,    -1,    19,    55,    -1,
      21,   115,    -1,    22,   115,    -1,    23,   118,    -1,    77,
      -1,    76,    77,    -1,    -1,    13,    47,    78,    79,    81,
      48,    -1,    -1,    80,    -1,    79,    80,    -1,    33,   118,
      -1,    31,   114,    -1,    82,    -1,    81,    82,    -1,    -1,
      14,    47,    83,    84,    86,    48,    -1,    -1,    85,    -1,
      84,    85,    -1,    18,   115,    -1,    31,   114,    -1,    87,
      -1,    86,    87,    -1,    -1,    15,    47,    88,    89,    48,
      -1,    -1,    90,    -1,    89,    90,    -1,    18,   115,    -1,
      31,   114,    -1,    92,    -1,    91,    92,    -1,    -1,    16,
      47,    93,    95,    94,    95,    48,    -1,    -1,    91,    -1,
      -1,    96,    -1,    95,    96,    -1,    18,   115,    -1,    15,
     115,    -1,    -1,    24,    49,    97,    99,    50,    -1,    -1,
      25,    49,    98,   101,    50,    -1,    31,   114,    -1,    32,
      49,   116,   116,    50,    -1,   103,    -1,   104,    -1,   108,
      -1,   110,    -1,    -1,   100,    -1,    99,   100,    -1,    26,
      -1,    27,    -1,    28,    -1,    -1,   102,    -1,   101,   102,
      -1,    29,    -1,    30,    -1,    34,    47,    35,    49,   116,
     116,   116,    50,    36,    49,   116,   116,   116,    50,    37,
      49,   116,   116,   116,    50,    48,    -1,    -1,    40,    47,
     105,   106,    48,    -1,    -1,   107,    -1,   106,   107,    -1,
      18,   115,    -1,    -1,    41,    47,   109,   112,    48,    -1,
      -1,    42,    47,   111,   112,    48,    -1,    -1,   113,    -1,
     112,   113,    -1,    18,   115,    -1,    49,   116,   116,   116,
     116,    50,    -1,    43,    -1,    44,    -1,   117,    -1,    45,
      -1,   118,    -1,    46,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   114,   114,   116,   117,   120,   120,   123,   128,   133,
     138,   143,   149,   151,   151,   154,   154,   155,   155,   158,
     158,   158,   160,   162,   162,   163,   163,   164,   164,   168,
     168,   172,   172,   172,   173,   176,   180,   180,   182,   181,
     188,   188,   189,   191,   192,   193,   194,   197,   197,   198,
     198,   201,   201,   201,   203,   204,   210,   210,   211,   211,
     214,   214,   214,   216,   217,   226,   226,   227,   227,   230,
     231,   231,   233,   234,   244,   244,   246,   245,   259,   259,
     261,   261,   262,   264,   265,   273,   273,   276,   275,   278,
     280,   282,   283,   284,   285,   287,   287,   287,   289,   290,
     291,   293,   293,   293,   295,   296,   298,   319,   319,   325,
     325,   326,   328,   330,   330,   336,   336,   342,   342,   342,
     344,   346,   354,   359,   360,   362,   363,   364
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
  "EQTOKEN_CONNECTION_LAUNCH_COMMAND", "EQTOKEN_SERVER", "EQTOKEN_CONFIG",
  "EQTOKEN_APPNODE", "EQTOKEN_NODE", "EQTOKEN_PIPE", "EQTOKEN_WINDOW",
  "EQTOKEN_CHANNEL", "EQTOKEN_COMPOUND", "EQTOKEN_CONNECTION",
  "EQTOKEN_NAME", "EQTOKEN_TYPE", "EQTOKEN_TCPIP", "EQTOKEN_HOSTNAME",
  "EQTOKEN_COMMAND", "EQTOKEN_TIMEOUT", "EQTOKEN_TASK", "EQTOKEN_FORMAT",
  "EQTOKEN_CLEAR", "EQTOKEN_DRAW", "EQTOKEN_READBACK", "EQTOKEN_COLOR",
  "EQTOKEN_DEPTH", "EQTOKEN_VIEWPORT", "EQTOKEN_RANGE", "EQTOKEN_DISPLAY",
  "EQTOKEN_WALL", "EQTOKEN_BOTTOM_LEFT", "EQTOKEN_BOTTOM_RIGHT",
  "EQTOKEN_TOP_LEFT", "EQTOKEN_SYNC", "EQTOKEN_LATENCY",
  "EQTOKEN_SWAPBARRIER", "EQTOKEN_OUTPUTFRAME", "EQTOKEN_INPUTFRAME",
  "EQTOKEN_STRING", "EQTOKEN_FLOAT", "EQTOKEN_INTEGER", "EQTOKEN_UNSIGNED",
  "'{'", "'}'", "'['", "']'", "$accept", "file", "global", "globals",
  "connectionType", "server", "@1", "configs", "config", "@2",
  "configAttributes", "configAttribute", "nodes", "node", "otherNode",
  "@3", "appNode", "@4", "nodeAttributes", "nodeAttribute", "connections",
  "connection", "@5", "connectionAttributes", "connectionAttribute",
  "pipes", "pipe", "@6", "pipeAttributes", "pipeAttribute", "windows",
  "window", "@7", "windowAttributes", "windowAttribute", "channels",
  "channel", "@8", "channelAttributes", "channelAttribute", "compounds",
  "compound", "@9", "compoundChildren", "compoundAttributes",
  "compoundAttribute", "@10", "@11", "compoundTasks", "compoundTask",
  "compoundFormats", "compoundFormat", "wall", "swapBarrier", "@12",
  "swapBarrierAttributes", "swapBarrierAttribute", "outputFrame", "@13",
  "inputFrame", "@14", "frameAttributes", "frameAttribute", "viewport",
  "STRING", "FLOAT", "INTEGER", "UNSIGNED", 0
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
     295,   296,   297,   298,   299,   300,   301,   123,   125,    91,
      93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    51,    52,    53,    53,    54,    54,    53,    53,    53,
      53,    53,    55,    57,    56,    58,    58,    60,    59,    61,
      61,    61,    62,    63,    63,    64,    64,    66,    65,    68,
      67,    69,    69,    69,    70,    71,    71,    71,    73,    72,
      74,    74,    74,    75,    75,    75,    75,    76,    76,    78,
      77,    79,    79,    79,    80,    80,    81,    81,    83,    82,
      84,    84,    84,    85,    85,    86,    86,    88,    87,    89,
      89,    89,    90,    90,    91,    91,    93,    92,    94,    94,
      95,    95,    95,    96,    96,    97,    96,    98,    96,    96,
      96,    96,    96,    96,    96,    99,    99,    99,   100,   100,
     100,   101,   101,   101,   102,   102,   103,   105,   104,   106,
     106,   106,   107,   109,   108,   111,   110,   112,   112,   112,
     113,   114,   115,   116,   116,   117,   117,   118
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
       1,     2,     2,     2,     1,     2,     0,     7,     0,     1,
       0,     1,     2,     2,     2,     0,     5,     0,     5,     2,
       5,     1,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     0,     1,     2,     1,     1,    21,     0,     5,     0,
       1,     2,     2,     0,     5,     0,     5,     0,     1,     2,
       2,     6,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     0,     4,
      12,     7,   122,     8,   127,     9,    10,    11,     1,     0,
       2,     5,     0,    13,     3,     6,     0,     0,     0,    15,
      17,    14,    16,    19,     0,     0,    20,    22,     0,     0,
      21,     0,    23,    26,    25,    29,    27,     0,    24,     0,
      74,    35,    35,    76,    18,    75,     0,    31,    36,    31,
      80,    38,     0,    32,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    78,    81,    91,    92,
      93,    94,    40,     0,    33,     0,    47,     0,    84,    83,
      85,    87,     0,    89,     0,     0,   107,   113,   115,    79,
      80,    82,     0,     0,     0,     0,     0,    41,    49,    30,
      48,    28,    95,   101,   123,   125,     0,   124,   126,     0,
       0,   109,   117,   117,     0,    43,    44,    45,    46,    39,
      42,    51,    98,    99,   100,     0,    96,   104,   105,     0,
     102,     0,     0,     0,     0,     0,   110,     0,     0,   118,
       0,    77,     0,     0,     0,    52,    86,    97,    88,   103,
       0,    90,     0,   112,   108,   111,   120,   114,   119,   116,
      55,    54,     0,    53,     0,    56,     0,     0,    58,    50,
      57,   121,     0,    60,     0,     0,     0,     0,    61,     0,
      63,    64,     0,    62,     0,    65,     0,    67,    59,    66,
       0,    69,     0,     0,     0,     0,    70,     0,    72,    73,
      68,    71,     0,     0,     0,     0,     0,     0,     0,   106
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     7,     8,    22,    11,    20,    26,    28,    29,    33,
      35,    36,    41,    42,    43,    52,    44,    51,    62,    63,
      57,    58,    82,   106,   107,    85,    86,   131,   154,   155,
     174,   175,   183,   187,   188,   194,   195,   201,   205,   206,
      49,    50,    60,   100,    76,    77,   112,   113,   135,   136,
     139,   140,    78,    79,   121,   145,   146,    80,   122,    81,
     123,   148,   149,    93,    13,   116,   117,   118
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -149
static const short int yypact[] =
{
     151,   -11,    26,    27,    29,    29,    27,    95,   118,   151,
    -149,  -149,  -149,  -149,  -149,  -149,  -149,  -149,  -149,    66,
    -149,  -149,     5,  -149,  -149,  -149,   130,   101,     9,  -149,
    -149,  -149,  -149,   121,    29,    28,  -149,  -149,   114,   115,
    -149,   103,  -149,  -149,  -149,  -149,  -149,   116,  -149,    10,
    -149,   147,   147,  -149,  -149,  -149,   119,   147,  -149,   147,
     111,  -149,   152,  -149,  -149,   152,    27,    27,   120,   122,
     123,   124,   127,   128,   129,   131,    76,  -149,  -149,  -149,
    -149,  -149,   109,   133,  -149,     1,  -149,     3,  -149,  -149,
    -149,  -149,    -2,  -149,    -2,   132,  -149,  -149,  -149,   154,
     111,  -149,    26,    27,    27,    29,    55,  -149,  -149,  -149,
    -149,  -149,    94,   104,  -149,  -149,    -2,  -149,  -149,    -2,
     134,   150,   159,   159,    48,  -149,  -149,  -149,  -149,  -149,
    -149,   106,  -149,  -149,  -149,    -5,  -149,  -149,  -149,    54,
    -149,    -2,   135,    -2,    27,    11,  -149,    27,    14,  -149,
      16,  -149,   123,    29,     4,  -149,  -149,  -149,  -149,  -149,
      -2,  -149,    -2,  -149,  -149,  -149,  -149,  -149,  -149,  -149,
    -149,  -149,   137,  -149,    -7,  -149,   136,    -2,  -149,  -149,
    -149,  -149,   138,     2,   145,    27,   123,    50,  -149,   140,
    -149,  -149,   143,  -149,     0,  -149,    -2,  -149,  -149,  -149,
      -2,    75,    -2,    27,   123,    -1,  -149,   141,  -149,  -149,
    -149,  -149,   155,   144,    -2,    -2,    -2,   146,   139,  -149
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -149,  -149,    90,  -149,    80,  -149,  -149,  -149,   166,  -149,
    -149,   160,  -149,   157,  -149,  -149,  -149,  -149,   142,   -38,
     148,    87,  -149,  -149,    93,   149,    62,  -149,  -149,    49,
    -149,    30,  -149,  -149,    15,  -149,    12,  -149,  -149,     6,
     153,   -44,  -149,  -149,   105,   -70,  -149,  -149,  -149,    72,
    -149,    69,  -149,  -149,  -149,  -149,    64,  -149,  -149,  -149,
    -149,    89,   -63,  -148,    -6,   -91,  -149,    -3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      17,    15,    16,   119,   170,    55,   101,   172,     1,     2,
       3,     4,     5,     6,    83,   192,    83,   203,   172,    27,
     185,   132,   133,   134,    84,   141,    47,    84,   142,   144,
     204,    37,   147,   186,   147,   152,     9,   153,   191,    38,
      39,   179,   114,   115,    14,   156,    10,   210,   198,   109,
     160,   111,   162,    24,   101,    55,   209,    31,    54,   164,
      88,    89,   167,    66,   169,   192,    67,    34,   185,   176,
      12,   177,    68,    69,   102,    14,   103,   104,   105,    70,
      71,   186,    72,   137,   138,   168,   182,   168,    73,    74,
      75,    66,    47,   203,    67,    18,   151,   126,   127,    21,
      68,    69,   128,   129,   158,   200,   204,    70,    71,   202,
      72,   207,    25,    23,    38,    39,    73,    74,    75,    47,
     132,   133,   134,   215,   216,   217,    66,    19,   102,    67,
     103,   104,   105,   137,   138,    68,    69,   152,   163,   153,
      27,   166,    70,    71,    64,    72,    64,   110,    30,   110,
     171,    73,    74,    75,     1,     2,     3,     4,     5,     6,
      34,    45,    46,    53,    56,    83,    61,   120,   144,    90,
      47,    91,    92,    94,    95,    96,    97,   147,    98,   190,
     108,   189,   125,   143,   178,   161,   181,   219,   184,   196,
     197,   212,   213,   214,    32,    40,   218,   208,    48,   130,
      59,    65,   193,   173,   180,   124,   199,   157,   159,   165,
       0,   211,   150,     0,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99
};

static const short int yycheck[] =
{
       6,     4,     5,    94,   152,    49,    76,    14,     3,     4,
       5,     6,     7,     8,    13,    15,    13,    18,    14,    10,
      18,    26,    27,    28,    62,   116,    16,    65,   119,    18,
      31,    34,    18,    31,    18,    31,    47,    33,   186,    11,
      12,    48,    44,    45,    46,    50,    20,    48,    48,    48,
     141,    48,   143,    48,   124,    99,   204,    48,    48,    48,
      66,    67,    48,    15,    48,    15,    18,    39,    18,   160,
      43,   162,    24,    25,    19,    46,    21,    22,    23,    31,
      32,    31,    34,    29,    30,   148,   177,   150,    40,    41,
      42,    15,    16,    18,    18,     0,    48,   103,   104,     9,
      24,    25,   105,    48,    50,   196,    31,    31,    32,   200,
      34,   202,    22,    47,    11,    12,    40,    41,    42,    16,
      26,    27,    28,   214,   215,   216,    15,     9,    19,    18,
      21,    22,    23,    29,    30,    24,    25,    31,   144,    33,
      10,   147,    31,    32,    57,    34,    59,    85,    47,    87,
     153,    40,    41,    42,     3,     4,     5,     6,     7,     8,
      39,    47,    47,    47,    17,    13,    47,    35,    18,    49,
      16,    49,    49,    49,    47,    47,    47,    18,    47,   185,
      47,    36,   102,    49,    47,    50,    50,    48,    50,    49,
      47,    50,    37,    49,    28,    35,    50,   203,    41,   106,
      52,    59,   187,   154,   174,   100,   194,   135,   139,   145,
      -1,   205,   123,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    52,    53,    47,
      20,    55,    43,   115,    46,   118,   118,   115,     0,     9,
      56,    53,    54,    47,    48,    53,    57,    10,    58,    59,
      47,    48,    59,    60,    39,    61,    62,   118,    11,    12,
      62,    63,    64,    65,    67,    47,    47,    16,    64,    91,
      92,    68,    66,    47,    48,    92,    17,    71,    72,    71,
      93,    47,    69,    70,    72,    69,    15,    18,    24,    25,
      31,    32,    34,    40,    41,    42,    95,    96,   103,   104,
     108,   110,    73,    13,    70,    76,    77,    76,   115,   115,
      49,    49,    49,   114,    49,    47,    47,    47,    47,    91,
      94,    96,    19,    21,    22,    23,    74,    75,    47,    48,
      77,    48,    97,    98,    44,    45,   116,   117,   118,   116,
      35,   105,   109,   111,    95,    55,   115,   115,   118,    48,
      75,    78,    26,    27,    28,    99,   100,    29,    30,   101,
     102,   116,   116,    49,    18,   106,   107,    18,   112,   113,
     112,    48,    31,    33,    79,    80,    50,   100,    50,   102,
     116,    50,   116,   115,    48,   107,   115,    48,   113,    48,
     114,   118,    14,    80,    81,    82,   116,   116,    47,    48,
      82,    50,   116,    83,    50,    18,    31,    84,    85,    36,
     115,   114,    15,    85,    86,    87,    49,    47,    48,    87,
     116,    88,   116,    18,    31,    89,    90,   116,   115,   114,
      48,    90,    50,    37,    49,   116,   116,   116,    50,    48
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

    { (yyval._connectionType) = eqNet::Connection::TYPE_TCPIP; ;}
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

    { config->setLatency( (yyvsp[0]._unsigned) ); ;}
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

    { connectionDescription->type = (yyvsp[0]._connectionType); ;}
    break;

  case 44:

    { connectionDescription->hostname = (yyvsp[0]._string); ;}
    break;

  case 45:

    { connectionDescription->launchCommand = (yyvsp[0]._string); ;}
    break;

  case 46:

    { connectionDescription->launchTimeout = (yyvsp[0]._unsigned); ;}
    break;

  case 49:

    { eqPipe = loader->createPipe(); ;}
    break;

  case 50:

    { node->addPipe( eqPipe ); eqPipe = NULL; ;}
    break;

  case 54:

    { eqPipe->setDisplay( (yyvsp[0]._unsigned) ); ;}
    break;

  case 55:

    {
            eqPipe->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], (int)(yyvsp[0]._viewport)[1],
                                                      (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
        ;}
    break;

  case 58:

    { window = loader->createWindow(); ;}
    break;

  case 59:

    { eqPipe->addWindow( window ); window = NULL; ;}
    break;

  case 63:

    { window->setName( (yyvsp[0]._string) ); ;}
    break;

  case 64:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                window->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0], 
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                window->setViewport( eq::Viewport((yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3])); 
        ;}
    break;

  case 67:

    { channel = loader->createChannel(); ;}
    break;

  case 68:

    { window->addChannel( channel ); channel = NULL; ;}
    break;

  case 72:

    { channel->setName( (yyvsp[0]._string) ); ;}
    break;

  case 73:

    {
            if( (yyvsp[0]._viewport)[2] > 1 || (yyvsp[0]._viewport)[3] > 1 )
                channel->setPixelViewport( eq::PixelViewport( (int)(yyvsp[0]._viewport)[0],
                                          (int)(yyvsp[0]._viewport)[1], (int)(yyvsp[0]._viewport)[2], (int)(yyvsp[0]._viewport)[3] ));
            else
                channel->setViewport(eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3]));
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

  case 83:

    { compound->setName( (yyvsp[0]._string) ); ;}
    break;

  case 84:

    {
         eqs::Channel* channel = config->findChannel( (yyvsp[0]._string) );
         if( !channel )
             yyerror( "No channel of the given name" );
         else
             compound->setChannel( channel );
    ;}
    break;

  case 85:

    { compound->setTasks( eqs::Compound::TASK_NONE ); ;}
    break;

  case 87:

    { compound->setFormats( eq::Frame::FORMAT_UNDEFINED ); ;}
    break;

  case 89:

    { compound->setViewport( eq::Viewport( (yyvsp[0]._viewport)[0], (yyvsp[0]._viewport)[1], (yyvsp[0]._viewport)[2], (yyvsp[0]._viewport)[3] )); ;}
    break;

  case 90:

    { compound->setRange( eq::Range( (yyvsp[-2]._float), (yyvsp[-1]._float) )); ;}
    break;

  case 98:

    { compound->enableTask( eqs::Compound::TASK_CLEAR ); ;}
    break;

  case 99:

    { compound->enableTask( eqs::Compound::TASK_DRAW ); ;}
    break;

  case 100:

    { compound->enableTask( eqs::Compound::TASK_READBACK ); ;}
    break;

  case 104:

    { compound->enableFormat( eq::Frame::FORMAT_COLOR ); ;}
    break;

  case 105:

    { compound->enableFormat( eq::Frame::FORMAT_DEPTH ); ;}
    break;

  case 106:

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
        compound->setWall( wall );
    ;}
    break;

  case 107:

    { swapBarrier = new eqs::SwapBarrier(); ;}
    break;

  case 108:

    { 
            compound->setSwapBarrier( swapBarrier );
            swapBarrier = NULL;
        ;}
    break;

  case 112:

    { swapBarrier->setName( (yyvsp[0]._string) ); ;}
    break;

  case 113:

    { frame = new eqs::Frame(); ;}
    break;

  case 114:

    { 
            compound->addOutputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 115:

    { frame = new eqs::Frame(); ;}
    break;

  case 116:

    { 
            compound->addInputFrame( frame );
            frame = NULL;
        ;}
    break;

  case 120:

    { frame->setName( (yyvsp[0]._string) ); ;}
    break;

  case 121:

    { 
         (yyval._viewport)[0] = (yyvsp[-4]._float);
         (yyval._viewport)[1] = (yyvsp[-3]._float);
         (yyval._viewport)[2] = (yyvsp[-2]._float);
         (yyval._viewport)[3] = (yyvsp[-1]._float);
     ;}
    break;

  case 122:

    {
         stringBuf = yytext;
         (yyval._string) = stringBuf.c_str(); 
     ;}
    break;

  case 123:

    { (yyval._float) = atof( yytext ); ;}
    break;

  case 124:

    { (yyval._float) = (yyvsp[0]._int); ;}
    break;

  case 125:

    { (yyval._int) = atoi( yytext ); ;}
    break;

  case 126:

    { (yyval._int) = (yyvsp[0]._unsigned); ;}
    break;

  case 127:

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

