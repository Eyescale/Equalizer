/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_CONFIG_H
#define IVS_SYSTEM_CONFIG_H

// compiler & architecture
#ifdef __GNUC__
#  define IVS_GCC
#  if defined(_X86) || defined(__i386) || defined(i386) || defined (__amd64)
#    define IVS_X86
#    define IVS_ALIGN __attribute__((aligned (16)))
#  endif
#elif defined(_MSC_VER)
#  define IVS_MSVC
#  if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
#    define IVS_X86
#    define IVS_ALIGN __declspec(align(16))
#  endif
#endif

// os
#if defined(__APPLE__) || defined(MACOSX)
#  define IVS_MAC
#elif defined(__linux)
#  define IVS_LINUX
#elif defined(_WIN32)
#  define NOMINMAX
#  define IVS_WIN32
#endif

// byte order
#if defined(IVS_MAC) || defined(IVS_LINUX)
#  ifdef IVS_MAC
#    include <machine/endian.h>
#  elif defined(IVS_LINUX)
#    include <endian.h>
#  endif
#  ifdef BYTE_ORDER
#    if BYTE_ORDER == BIG_ENDIAN
#      define IVS_BIG_ENDIAN
#    elif BYTE_ORDER == LITTLE_ENDIAN
#      define IVS_LITTLE_ENDIAN
#    endif
#  endif
#elif defined(IVS_WIN32)
#  define IVS_LITTLE_ENDIAN
#endif

#endif
