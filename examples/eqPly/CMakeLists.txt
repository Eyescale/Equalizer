# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(KD_HEADERS
    ply.h
    vertexBufferBase.h
    vertexBufferData.h
    vertexBufferDist.h
    vertexBufferLeaf.h
    vertexBufferNode.h
    vertexBufferRoot.h
    vertexBufferState.h
    vertexData.h
)
set(KD_SOURCES
    plyfile.cpp
    vertexBufferBase.cpp
    vertexBufferDist.cpp
    vertexBufferLeaf.cpp
    vertexBufferNode.cpp
    vertexBufferRoot.cpp
    vertexBufferState.cpp
    vertexData.cpp
)
source_group(kdTree FILES ${KD_HEADERS} ${KD_SOURCES})

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
    tracker.h
    typedefs.h
    view.h
    window.h
    ${KD_HEADERS}
  SOURCES
    cameraAnimation.cpp
    channel.cpp
    config.cpp
    configEvent.cpp
    eqPly.cpp
    error.cpp
    frameData.cpp
    initData.cpp
    localInitData.cpp
    main.cpp
    node.cpp
    pipe.cpp
    tracker.cpp
    view.cpp
    window.cpp
    ${KD_SOURCES}
  SHADERS
    vertexShader.glsl
    fragmentShader.glsl
  LINK_LIBRARIES
    ${EQUALIZER_ADMIN_LIBRARY}
  )

# create and install .bin files
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BIN_POSTFIX "64.bin")
else()
  set(BIN_POSTFIX "32.bin")
endif()
if(EQ_BIG_ENDIAN)
  set(BIN_POSTFIX "be${BIN_POSTFIX}")
else()
  set(BIN_POSTFIX "le${BIN_POSTFIX}")
endif()

if(NOT CMAKE_BUILD_WITH_INSTALL_RPATH) # MacPorts WAR
  set(PLY_FILES rockerArm.ply screwdriver.ply)
endif()
foreach(PLY_FILE ${PLY_FILES})
  set(BIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${PLY_FILE}.${BIN_POSTFIX}")
  add_custom_command(OUTPUT "${BIN_FILE}"
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/eqPlyConverter"
    "${CMAKE_CURRENT_SOURCE_DIR}/${PLY_FILE}"
    MAIN_DEPENDENCY ${PLY_FILE}
    DEPENDS eqPlyConverter
    COMMENT "Creating eqPly kd-Tree for ${PLY_FILE}"
    )
  add_custom_target("${PLY_FILE}.${BIN_POSTFIX}" ALL DEPENDS "${BIN_FILE}")
  install(FILES ${PLY_FILE} ${BIN_FILE} DESTINATION share/Equalizer/data
    COMPONENT data)
endforeach()
install(FILES logo.rgb DESTINATION share/Equalizer/data COMPONENT data)
