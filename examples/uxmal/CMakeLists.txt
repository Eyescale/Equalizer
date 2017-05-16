# Copyright (c) 2017 Stefan.Eilemann@epfl.ch

include(zerobufGenerateCxx)
set(UXMAL_DIR ${PROJECT_BINARY_DIR}/include/uxmal)
set(UXMAL_FBS
  ${CMAKE_CURRENT_SOURCE_DIR}/aabb.fbs
  ${CMAKE_CURRENT_SOURCE_DIR}/frustum.fbs)
zerobuf_generate_cxx(UXMAL ${UXMAL_DIR} ${UXMAL_FBS})

set(UXMAL_LINK_LIBRARIES ZeroBuf ZeroEQ)
set(UXMAL_PUBLIC_HEADERS ${CULLBUG_HEADERS})
common_library(uxmal)
target_compile_definitions(uxmal PUBLIC UXMAL)

set(EQUXMAL_PUBLIC_HEADERS ${EQHELLO_HEADERS})
set(EQUXMAL_SOURCES uxmal.cpp)
set(EQUXMAL_LINK_LIBRARIES Sequel uxmal)

common_application(eqUxmal GUI EXAMPLE)
