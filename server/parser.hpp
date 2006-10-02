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
     EQTOKEN_SERVER = 265,
     EQTOKEN_CONFIG = 266,
     EQTOKEN_APPNODE = 267,
     EQTOKEN_NODE = 268,
     EQTOKEN_PIPE = 269,
     EQTOKEN_WINDOW = 270,
     EQTOKEN_ATTRIBUTES = 271,
     EQTOKEN_HINTS = 272,
     EQTOKEN_STEREO = 273,
     EQTOKEN_ON = 274,
     EQTOKEN_OFF = 275,
     EQTOKEN_AUTO = 276,
     EQTOKEN_CHANNEL = 277,
     EQTOKEN_COMPOUND = 278,
     EQTOKEN_CONNECTION = 279,
     EQTOKEN_NAME = 280,
     EQTOKEN_TYPE = 281,
     EQTOKEN_TCPIP = 282,
     EQTOKEN_HOSTNAME = 283,
     EQTOKEN_COMMAND = 284,
     EQTOKEN_TIMEOUT = 285,
     EQTOKEN_TASK = 286,
     EQTOKEN_EYE = 287,
     EQTOKEN_FORMAT = 288,
     EQTOKEN_CLEAR = 289,
     EQTOKEN_DRAW = 290,
     EQTOKEN_READBACK = 291,
     EQTOKEN_CYCLOP = 292,
     EQTOKEN_LEFT = 293,
     EQTOKEN_RIGHT = 294,
     EQTOKEN_COLOR = 295,
     EQTOKEN_DEPTH = 296,
     EQTOKEN_VIEWPORT = 297,
     EQTOKEN_RANGE = 298,
     EQTOKEN_DISPLAY = 299,
     EQTOKEN_WALL = 300,
     EQTOKEN_BOTTOM_LEFT = 301,
     EQTOKEN_BOTTOM_RIGHT = 302,
     EQTOKEN_TOP_LEFT = 303,
     EQTOKEN_SYNC = 304,
     EQTOKEN_LATENCY = 305,
     EQTOKEN_SWAPBARRIER = 306,
     EQTOKEN_OUTPUTFRAME = 307,
     EQTOKEN_INPUTFRAME = 308,
     EQTOKEN_STRING = 309,
     EQTOKEN_FLOAT = 310,
     EQTOKEN_INTEGER = 311,
     EQTOKEN_UNSIGNED = 312
   };
#endif
#define EQTOKEN_GLOBAL 258
#define EQTOKEN_CONNECTION_TYPE 259
#define EQTOKEN_CONNECTION_HOSTNAME 260
#define EQTOKEN_CONNECTION_TCPIP_PORT 261
#define EQTOKEN_CONNECTION_LAUNCH_TIMEOUT 262
#define EQTOKEN_CONNECTION_LAUNCH_COMMAND 263
#define EQTOKEN_WINDOW_IATTR_HINTS_STEREO 264
#define EQTOKEN_SERVER 265
#define EQTOKEN_CONFIG 266
#define EQTOKEN_APPNODE 267
#define EQTOKEN_NODE 268
#define EQTOKEN_PIPE 269
#define EQTOKEN_WINDOW 270
#define EQTOKEN_ATTRIBUTES 271
#define EQTOKEN_HINTS 272
#define EQTOKEN_STEREO 273
#define EQTOKEN_ON 274
#define EQTOKEN_OFF 275
#define EQTOKEN_AUTO 276
#define EQTOKEN_CHANNEL 277
#define EQTOKEN_COMPOUND 278
#define EQTOKEN_CONNECTION 279
#define EQTOKEN_NAME 280
#define EQTOKEN_TYPE 281
#define EQTOKEN_TCPIP 282
#define EQTOKEN_HOSTNAME 283
#define EQTOKEN_COMMAND 284
#define EQTOKEN_TIMEOUT 285
#define EQTOKEN_TASK 286
#define EQTOKEN_EYE 287
#define EQTOKEN_FORMAT 288
#define EQTOKEN_CLEAR 289
#define EQTOKEN_DRAW 290
#define EQTOKEN_READBACK 291
#define EQTOKEN_CYCLOP 292
#define EQTOKEN_LEFT 293
#define EQTOKEN_RIGHT 294
#define EQTOKEN_COLOR 295
#define EQTOKEN_DEPTH 296
#define EQTOKEN_VIEWPORT 297
#define EQTOKEN_RANGE 298
#define EQTOKEN_DISPLAY 299
#define EQTOKEN_WALL 300
#define EQTOKEN_BOTTOM_LEFT 301
#define EQTOKEN_BOTTOM_RIGHT 302
#define EQTOKEN_TOP_LEFT 303
#define EQTOKEN_SYNC 304
#define EQTOKEN_LATENCY 305
#define EQTOKEN_SWAPBARRIER 306
#define EQTOKEN_OUTPUTFRAME 307
#define EQTOKEN_INPUTFRAME 308
#define EQTOKEN_STRING 309
#define EQTOKEN_FLOAT 310
#define EQTOKEN_INTEGER 311
#define EQTOKEN_UNSIGNED 312




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
} YYSTYPE;
/* Line 1249 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE eqLoader_lval;



