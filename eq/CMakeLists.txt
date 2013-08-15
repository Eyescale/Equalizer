# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_MX_INCLUDE_DIRS})
if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
endif()

add_subdirectory(fabric)
add_subdirectory(client)
add_subdirectory(admin)
add_subdirectory(server)

install(FILES eq.h defines.h DESTINATION include/eq COMPONENT dev)
