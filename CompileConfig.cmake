# Copyright (c) 2012-2013 Fabien Delalondre <fabien.delalondre@epfl.ch>
# Adding support for compiler optimization and definition of release configs
# according to chosen compiler 
# Supported compilers are XL, Intel, Clang and GNU

# Compiler settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "XL")
  set(CMAKE_COMPILER_IS_XLCXX ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_COMPILER_IS_INTEL ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_COMPILER_IS_CLANG ON)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_COMPILER_IS_GNUCXX_PURE ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
  include(${CMAKE_CURRENT_LIST_DIR}/CompilerVersion.cmake)
  COMPILER_DUMPVERSION(GCC_COMPILER_VERSION)
  if(GCC_COMPILER_VERSION VERSION_LESS 4.1)
    message(ERROR "GCC 4.1 or later required, found ${GCC_COMPILER_VERSION}")
  endif()
  set(COMMON_GCC_FLAGS "-Wall -Wextra -Winvalid-pch -Winit-self -Wno-unknown-pragmas -Wno-unused-parameter")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_GCC_FLAGS} ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_GCC_FLAGS} -Wnon-virtual-dtor -Wsign-promo")
  if(GCC_COMPILER_VERSION VERSION_GREATER 4.1) # < 4.2 doesn't know -isystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wuninitialized")
  if(NOT WIN32 AND NOT XCODE_VERSION AND NOT RELEASE_VERSION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  endif()
  if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  endif()
elseif(CMAKE_COMPILER_IS_INTEL)
  # default 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-unknown-pragmas")

  # optimized: flags + trying to link against MKL library
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  # adding MKL if available
  find_package(MKL)
  # FindMKL.cmake implementation not ideal, no FOUND variable declared. 
  # workaround by testing if the include is set. Will fork findMKL.cmake to fix it + checking library name
  # lmkl_intel_ilp64
  if(NOT ${MKL_INCLUDE_DIRS} STREQUAL "")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMKL_ILP64")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDMKL_ILP64")
    include_directories(${MKL_INCLUDE_DIRS})
    link_directories(${MKL_LIBRARIES})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MKL_LIBRARIES}")
  end()

elseif(CMAKE_COMPILER_IS_XLCXX)
  # default
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qstrict")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qstrict") 

  # optimized: flags + trying to link against MASS/MASSV libraries
  # adding -qstrict to avoid altering semantics
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DBGQ -O3 -qarch=qp -q64 -qhot=level=1 -qsimd=auto -qstrict ")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DBGQ -O3 -qarch=qp -q64 -qhot=level=1 -qsimd=auto -qstrict ")

  # Adding MASS/MASSV libraries: no find cmake script
  # adding mass and massv libraries should not be necessary 
  # since it should already be added by the compiler. 
  # adding it on the first pass anyway
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmass -lmassv")
endif()

