# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

find_package(CUDA REQUIRED)

include_directories(${CUDA_INCLUDE_DIRS})

cuda_compile(NBODY_FILES nbody_kernel.cu nbody.cu)

add_executable(eqNBody
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

target_link_libraries(eqNBody ${EQ_LIBRARIES} ${CUDA_LIBRARIES})

install(TARGETS eqNBody DESTINATION bin)
