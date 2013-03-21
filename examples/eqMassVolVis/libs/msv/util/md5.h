// MD5.CC - source code for the C++/object oriented translation and
//          modification of MD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug

// This translation/ modification is provided "as is," without express or
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that MD5 will do what you think
// it does; (2) that this translation/ modification is accurate; or (3) that
// this software is "merchantible."  (Language for this disclaimer partially
// copied from the disclaimer below).

/* based on:

   MD5.H - header file for MD5C.C
   MDDRIVER.C - test driver for MD2, MD4 and MD5

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#ifndef UTIL_MD5_H
#define UTIL_MD5_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <stdint.h>

class MD5 {

private:
// first, some types:
  typedef uint32_t      uint4; // assumes integer is 4 words long
  typedef unsigned char uint1; // assumes char is 1 word long

public:
// methods for controlled operation:
  MD5              ();  // simple initializer
  void  update     ( const unsigned char *input, uint4 input_length );
  void  finalize   ();

  char *compute( const unsigned char *input, uint4 input_length )
        { update( input, input_length ); finalize(); return hex_digest(); }


// methods to acquire finalized result
  unsigned char *raw_digest ();  // digest as a 16-byte binary array
           char *hex_digest ();  // digest as a 33-byte ascii-hex string

  friend std::ostream& operator<<( std::ostream&, MD5 context );

private:
// next, the private data:
  uint4 _state[4];
  uint4 _count[2];     // number of *bits*, mod 2^64
  uint1 _buffer[64];   // input buffer
  uint1 _digest[16];
  uint1 _finalized;

// last, the private methods, mostly static:
  void _init();                           // called by all constructors
  void _transform( const uint1 *buffer ); // does the real update work.  Note
                                         // that length is implied to be 64.

  static void _encode( uint1 *dest, const uint4 *src, uint4 length );
  static void _decode( uint4 *dest, const uint1 *src, uint4 length );

  static inline uint4  _rotate_left ( uint4 x, uint4 n );
  static inline uint4  _F           ( uint4 x, uint4 y, uint4 z );
  static inline uint4  _G           ( uint4 x, uint4 y, uint4 z );
  static inline uint4  _H           ( uint4 x, uint4 y, uint4 z );
  static inline uint4  _I           ( uint4 x, uint4 y, uint4 z );
  static inline void   _FF( uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
  static inline void   _GG( uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
  static inline void   _HH( uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );
  static inline void   _II( uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac );

};

#endif // UTIL_MD5_H
