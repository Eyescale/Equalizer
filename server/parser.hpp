/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE eqLoader_lval;

