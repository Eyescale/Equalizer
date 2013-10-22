# Copyright (c) 2013 Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
#
# - Try to find Google's gmock
# This module defines
#
# GMOCK_FOUND
# GMOCK_INCLUDE_DIRS
# GMOCK_LIBRARIES
# GTEST_LIBRARIES

find_path(GMOCK_INCLUDE_DIR gmock/gmock.h)
find_path(GTEST_INCLUDE_DIR gtest/gtest.h)

find_library(GMOCK_LIB NAMES libgmock.a)
find_library(GMOCK_LIB_MAIN NAMES libgmock_main.a)

set(GMOCK_LIBRARIES ${GMOCK_LIB} ${GMOCK_LIB_MAIN})
set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR})
set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gmock DEFAULT_MSG
				 GMOCK_LIB
				 GMOCK_LIB_MAIN
				 GMOCK_INCLUDE_DIR
				 GTEST_INCLUDE_DIR
)

mark_as_advanced(GMOCK_LIB 
		 GMOCK_LIB_MAIN
		 GMOCK_INCLUDE_DIR
		 GTEST_INCLUDE_DIR
)
