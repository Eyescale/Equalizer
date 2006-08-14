/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
     EQTOKEN_CLEAR = 280,
     EQTOKEN_DRAW = 281,
     EQTOKEN_VIEWPORT = 282,
     EQTOKEN_RANGE = 283,
     EQTOKEN_DISPLAY = 284,
     EQTOKEN_WALL = 285,
     EQTOKEN_BOTTOM_LEFT = 286,
     EQTOKEN_BOTTOM_RIGHT = 287,
     EQTOKEN_TOP_LEFT = 288,
     EQTOKEN_SYNC = 289,
     EQTOKEN_LATENCY = 290,
     EQTOKEN_SWAPBARRIER = 291,
     EQTOKEN_OUTPUTFRAME = 292,
     EQTOKEN_INPUTFRAME = 293,
     EQTOKEN_STRING = 294,
     EQTOKEN_FLOAT = 295,
     EQTOKEN_INTEGER = 296,
     EQTOKEN_UNSIGNED = 297
   };
#endif
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
#define EQTOKEN_CLEAR 280
#define EQTOKEN_DRAW 281
#define EQTOKEN_VIEWPORT 282
#define EQTOKEN_RANGE 283
#define EQTOKEN_DISPLAY 284
#define EQTOKEN_WALL 285
#define EQTOKEN_BOTTOM_LEFT 286
#define EQTOKEN_BOTTOM_RIGHT 287
#define EQTOKEN_TOP_LEFT 288
#define EQTOKEN_SYNC 289
#define EQTOKEN_LATENCY 290
#define EQTOKEN_SWAPBARRIER 291
#define EQTOKEN_OUTPUTFRAME 292
#define EQTOKEN_INPUTFRAME 293
#define EQTOKEN_STRING 294
#define EQTOKEN_FLOAT 295
#define EQTOKEN_INTEGER 296
#define EQTOKEN_UNSIGNED 297




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    const char*             _string;
    int                     _int;
    unsigned                _unsigned;
    float                   _float;
    eqNet::Connection::Type _connectionType;
    float                   _viewport[4];
} YYSTYPE;
/* Line 1318 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE eqLoader_lval;



