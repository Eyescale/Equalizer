# Original file shamelessly copied from https://gforge.inria.fr/plugins/scmsvn/viewcvs.php/trunk/CMake/FindOmniORB.cmake?rev=501&root=redgrid&view=markup

#
# Find the omniORB libraries and include dir
#

# OMNIORB4_INCLUDE_DIR  - Directories to include to use omniORB
# OMNIORB4_LIBRARIES    - Files to link against to use omniORB
# OMNIORB4_IDL_COMPILER
# OMNIORB4_FOUND        - When false, don't try to use omniORB
# OMNIORB4_DIR          - (optional) Suggested installation directory to search
#
# OMNIORB4_DIR can be used to make it simpler to find the various include
# directories and compiled libraries when omniORB was not installed in the
# usual/well-known directories (e.g. because you made an in tree-source
# compilation or because you installed it in an "unusual" directory).
# Just set OMNIORB4_DIR it to your specific installation directory
#

if(OmniORB_FIND_REQUIRED)
  set(_output_type FATAL_ERROR)
else()
  set(_output_type STATUS)
endif()

find_path(OMNIORB4_INCLUDE_DIR omniORB4/CORBA.h
  PATHS
  ${OMNIORB4_DIR}/include
  /usr/include
  /usr/local/include
  /opt/include
)

set(OMNIORB4_DEFAULT_LIB_PATH /usr/lib /usr/local/lib )
#### For the list of required libraries for omniORB see
# http://www.omniorb-support.com/pipermail/omniorb-list/2005-May/026666.html
# Basically, look at
#  - omniORB-4.0.5/README.*
#  - omniORB-4.0.5/readmes/*
# Platfrom dependencies might (only?) happen for Win32/VC++ (sigh):
# "Because of a bug in VC++, unless you require the dynamic interfaces, it
#  is suggested that you use a smaller stub library called msvcstub.lib."

find_library(OMNIORB4_LIBRARY_omniORB4 omniORB4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
        /opt/lib
)

find_library(OMNIORB4_LIBRARY_omnithread omnithread
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
)

find_library(OMNIORB4_LIBRARY_COS4 COS4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
        /opt/lib
)

find_library(OMNIORB4_LIBRARY_COSDynamic4 COSDynamic4
  PATHS ${OMNIORB4_DIR}/lib
        ${OMNIORB4_DEFAULT_LIB_PATH}
        /opt/lib
)

find_program(OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS ${OMNIORB4_DIR}/bin
        /usr/bin
        /usr/local/bin
        /opt/bin
  DOC "What is the path where omniidl (the idl compiler) can be found"
)

mark_as_advanced (OMNIORB4_INCLUDE_DIR)
mark_as_advanced (OMNIORB4_LIBRARY_omniORB4)
mark_as_advanced (OMNIORB4_LIBRARY_omnithread)
mark_as_advanced (OMNIORB4_LIBRARY_COS4)
mark_as_advanced (OMNIORB4_LIBRARY_COSDynamic4)
mark_as_advanced (OMNIORB4_IDL_COMPILER)
  
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OmniORB4 DEFAULT_MSG
  OMNIORB4_INCLUDE_DIR OMNIORB4_LIBRARY_omniORB4 OMNIORB4_LIBRARY_omnithread
  OMNIORB4_LIBRARY_COS4 OMNIORB4_LIBRARY_COSDynamic4 OMNIORB4_IDL_COMPILER)

if(OMNIORB4_FOUND)
  set(OMNIORB4_LIBRARIES ${OMNIORB4_LIBRARY_omniORB4}
                         ${OMNIORB4_LIBRARY_omnithread}
                         ${OMNIORB4_LIBRARY_COS4}
                         ${OMNIORB4_LIBRARY_COSDynamic4})
else()
  message(${_output_type} "Missing Corba OmniORB")
endif()
