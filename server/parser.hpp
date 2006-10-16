/* A Bison parser, made by GNU Bison 2.2.  */

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
/* Line 1528 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE eqLoader_lval;

