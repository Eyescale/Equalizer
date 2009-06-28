/* Eq-Specific Defines */
#ifndef EQ_DEFINES_H
#define EQ_DEFINES_H
#ifndef WGL
#  define WGL
#endif
#ifndef WIN32
#  define WIN32
#endif
#ifndef WIN32_VC
#  define WIN32_VC
#endif
#ifndef WIN32_API
#  define WIN32_API
#endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef GLEW_MX
#  define GLEW_MX
#endif
#if 0
#  ifndef EQ_WIN32_SDP_JOIN_WAR // see thread.cpp
#    define EQ_WIN32_SDP_JOIN_WAR
#  endif
#endif
#ifdef _OPENMP
#  define EQ_USE_OPENMP
#endif
#ifndef EQ_CHECK_THREADSAFETY
#  define EQ_CHECK_THREADSAFETY
#endif
#ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN
#endif
#endif // EQ_DEFINES_H
