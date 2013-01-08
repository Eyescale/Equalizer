# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/FindVMMLIB)
include(FindLibraryPackage)
include(FindPackageHandleStandardArgs)

find_library_package(VMMLIB INCLUDE vmmlib NO_LIBRARY)
find_package_handle_standard_args(VMMLIB DEFAULT_MSG VMMLIB_INCLUDE_DIRS)

if(VMMLIB_FOUND)
  set(VMMLIB_DEB_DEPENDENCIES)
  set(VMMLIB_DEB_BUILD_DEPENDENCIES "vmmlib1-dev")
endif()
