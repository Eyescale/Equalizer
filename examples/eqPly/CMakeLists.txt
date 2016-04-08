# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

set(EQPLY_LINK_LIBRARIES Equalizer EqualizerAdmin triply
  ${Boost_PROGRAM_OPTIONS_LIBRARY})
if(EQ_QT_USED)
  list(APPEND EQPLY_LINK_LIBRARIES Qt5::Widgets ${X11_LIBRARIES})
endif()

set(EQPLY_HEADERS
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
  window.h)

set(EQPLY_SOURCES
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
  window.cpp)

set(EQPLY_SHADERS vertexShader.glsl fragmentShader.glsl)
set(EQPLY_DATA logo.rgb)
common_application(eqPly GUI EXAMPLE)

# create and install .bin files
function(INSTALL_PLY IN OUT) # + command
  set(BIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${OUT}")
  add_custom_command(OUTPUT ${BIN_FILE}
    COMMAND ${ARGN} "${CMAKE_CURRENT_SOURCE_DIR}/${IN}"
    COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE}
                                     ${CMAKE_BINARY_DIR}/share/Equalizer/data
    DEPENDS "${IN}" eqPlyConverter
    COMMENT "Creating kd-Tree for ${IN}"
    )
  add_custom_target(${OUT} ALL DEPENDS ${BIN_FILE})
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
  elseif(NOT MSVC)
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
  file(COPY ${PLY_FILE} DESTINATION ${CMAKE_BINARY_DIR}/share/Equalizer/data)
  install(FILES ${PLY_FILE} DESTINATION share/Equalizer/data COMPONENT apps)
endforeach()
