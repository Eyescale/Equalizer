/* Eq-Specific Defines */

#ifndef EQBASE_DEFINES_MSC_H
#define EQBASE_DEFINES_MSC_H
#ifndef WGL
#  define WGL
#endif
#ifndef WIN32
#  define WIN32
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
#ifdef _OPENMP
#  define EQ_USE_OPENMP
#endif
#ifndef EQ_USE_MAGELLAN
#  define EQ_USE_MAGELLAN
#endif
#ifndef EQ_PGM
#  define EQ_PGM
#endif
#ifndef LITTLE_ENDIAN
#  define LITTLE_ENDIAN
#endif

#if 1
#  ifndef EQ_USE_BOOST
#    define EQ_USE_BOOST
#  endif
#endif

#if 0 // Enable for IB builds (needs WinOF 2.0 installed)
#  ifndef EQ_INFINIBAND
#    define EQ_INFINIBAND
#  endif
#endif

#endif // EQBASE_DEFINES_MSC_H
