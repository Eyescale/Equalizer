# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2014 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

eq_add_example(eqPly
  HEADERS
    cameraAnimation.h
    channel.h
    config.h
    configEvent.h
    eqPly.h
    error.h
    frameData.h
    initData.h
    localInitData.h
    modelAssigner.h
    node.h
    node.h
    pipe.h
    vertexBufferState.h
    view.h
    window.h
  SOURCES
    cameraAnimation.cpp
    channel.cpp
    config.cpp
    eqPly.cpp
    error.cpp
    frameData.cpp
    initData.cpp
    localInitData.cpp
    main.cpp
    node.cpp
    pipe.cpp
    view.cpp
    window.cpp
  SHADERS
    vertexShader.glsl
    fragmentShader.glsl
  LINK_LIBRARIES
    ${EQUALIZER_ADMIN_LIBRARY} triply ${QT_LIBRARIES}
  )

install(FILES logo.rgb DESTINATION share/Equalizer/data COMPONENT apps)

if(MSVC) # the custom command below doesn't work on VS
  return()
endif()

# create and install .bin files
function(INSTALL_PLY IN OUT) # + command
  set(BIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${OUT}")
  add_custom_target(${OUT} ALL
    COMMAND ${ARGN} "${CMAKE_CURRENT_SOURCE_DIR}/${IN}"
    DEPENDS "${IN}" eqPlyConverter
    COMMENT "Creating eqPly kd-Tree for ${IN}"
    )
  install(FILES ${OUT} DESTINATION share/Equalizer/data COMPONENT apps)
endfunction()

set(PLY_FILES rockerArm.ply screwdriver.ply)
foreach(PLY_FILE ${PLY_FILES})
  if(APPLE)
    if(NOT CMAKE_BUILD_WITH_INSTALL_RPATH) # MacPorts WAR
      foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
        if(${ARCH} MATCHES "64")
          set(BIN_POSTFIX "64.bin")
        else()
          set(BIN_POSTFIX "32.bin")
        endif()
        if(${ARCH} MATCHES "ppc")
          set(BIN_POSTFIX "be${BIN_POSTFIX}")
        else()
          set(BIN_POSTFIX "le${BIN_POSTFIX}")
        endif()
        install_ply("${PLY_FILE}" "${PLY_FILE}.${BIN_POSTFIX}"
          arch -${ARCH} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eqPlyConverter")
      endforeach()
    endif()
  else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(BIN_POSTFIX "64.bin")
    else()
      set(BIN_POSTFIX "32.bin")
    endif()
    if(BIGENDIAN)
      set(BIN_POSTFIX "be${BIN_POSTFIX}")
    else()
      set(BIN_POSTFIX "le${BIN_POSTFIX}")
    endif()
    install_ply("${PLY_FILE}" "${PLY_FILE}.${BIN_POSTFIX}"
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eqPlyConverter")
  endif()
  install(FILES ${PLY_FILE} DESTINATION share/Equalizer/data COMPONENT apps)
endforeach()
install(FILES logo.rgb DESTINATION share/Equalizer/data COMPONENT apps)
