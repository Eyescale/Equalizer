# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(CUDA)

if(NOT CUDA_FOUND)
  message(WARNING "CUDA not found: Example 'eqNBody' will be skipped!")
  return()
endif(NOT CUDA_FOUND)

include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
set(CUDA_PROPAGATE_HOST_FLAGS OFF)

cuda_compile(NBODY_FILES nbody.cu)

add_executable(eqNBody MACOSX_BUNDLE
  ${NBODY_FILES}
  channel.cpp
  client.cpp
  config.cpp
  configEvent.cpp
  controller.cpp
  frameData.cpp
  initData.cpp
  main.cpp
  node.cpp
  pipe.cpp
  render_particles.cpp
  sharedData.cpp
  sharedDataProxy.cpp
  window.cpp
  )

target_link_libraries(eqNBody Equalizer ${CUDA_LIBRARIES})

install(TARGETS eqNBody DESTINATION bin)
