 
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef COBASE_COMPILER_H
#define COBASE_COMPILER_H

#ifdef _MSC_VER
/** Declare and align a variable to a 8-byte boundary. */
#  define EQ_ALIGN8( var )  __declspec (align (8)) var;
/** Declare and align a variable to a 16-byte boundary. */
#  define EQ_ALIGN16( var ) __declspec (align (16)) var;
#else
/** Declare and align a variable to a 8-byte boundary. */
#  define EQ_ALIGN8( var )  var __attribute__ ((aligned (8)));
/** Declare and align a variable to a 16-byte boundary. */
#  define EQ_ALIGN16( var ) var __attribute__ ((aligned (16)));
#endif

#ifdef __GNUC__
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 0)) )
#    define EQ_GCC_4_0_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)) )
#    define EQ_GCC_4_1_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2)) )
#    define EQ_GCC_4_2_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)) )
#    define EQ_GCC_4_3_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4)) )
#    define EQ_GCC_4_4_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) )
#    define EQ_GCC_4_5_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) )
#    define EQ_GCC_4_6_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)) )
#    define EQ_GCC_4_7_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) )
#    define EQ_GCC_4_8_OR_LATER
#  endif
#  if (( __GNUC__ > 4 ) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) )
#    define EQ_GCC_4_9_OR_LATER
#  endif
#endif // GCC

#endif //COBASE_COMPILER_H
