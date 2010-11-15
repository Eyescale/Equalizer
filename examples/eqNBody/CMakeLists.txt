# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

if(NOT CUDA_FOUND)
  message(STATUS "CUDA not found: Example 'eqNBody' will be skipped")
  return()
endif(NOT CUDA_FOUND)

cuda_compile(NBODY_FILES nbody.cu)

eq_add_example(eqNBody
  HEADERS
    channel.h
    client.h
    config.h
    configEvent.h
    controller.h
    frameData.h
    initData.h
    nbody.h
    node.h
    pipe.h
    render_particles.h
    sharedData.h
    sharedDataProxy.h
    window.h
  SOURCES
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
  LINK_LIBRARIES
    Equalizer
    ${CUDA_LIBRARIES}
  )
