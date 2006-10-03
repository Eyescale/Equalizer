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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
} YYSTYPE;
/* Line 1447 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE eqLoader_lval;



