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
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
//#ifndef EQ_BUILD_FOR_SDP
//#  define EQ_BUILD_FOR_SDP
//#endif
#ifdef EQ_BUILD_FOR_SDP
#  ifndef EQ_WIN32_SDP_JOIN_WAR // see thread.cxx
#    define EQ_WIN32_SDP_JOIN_WAR
#  endif
#else
#  ifndef EQ_USE_COMPRESSION
#    define EQ_USE_COMPRESSION
#  endif
#endif
#ifndef EQ_CHECK_THREADSAFETY
#  define EQ_CHECK_THREADSAFETY
#endif
#ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN
#endif
#endif // EQ_DEFINES_H
