# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

find_package(VMMLIB REQUIRED)
include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_MX_INCLUDE_DIRS}
                           ${VMMLIB_INCLUDE_DIRS})
if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
endif()

add_subdirectory(fabric)
add_subdirectory(client)
add_subdirectory(admin)
add_subdirectory(server)

install(FILES eq.h DESTINATION include/eq COMPONENT eqdev)

configure_file(Equalizer.pc.in ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Equalizer.pc
        DESTINATION ${LIBRARY_DIR}/pkgconfig COMPONENT eqlib)

