
# Copyright (c) 2011-2013 Stefan Eilemann <eile@eyescale.ch>

include(configure.cmake)
include(files.cmake)

add_library(EqualizerFabric SHARED
  ${EQ_FABRIC_PUBLIC_HEADERS} ${EQ_FABRIC_SOURCES} ${EQ_FABRIC_HEADERS})
target_link_libraries(EqualizerFabric ${COLLAGE_LIBRARIES})
set_target_properties(EqualizerFabric
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

install(FILES ${EQ_FABRIC_PUBLIC_HEADERS}
  DESTINATION include/eq/fabric COMPONENT eqdev)

if (MSVC)
  install(FILES ${CMAKE_BINARY_DIR}/bin/Debug/EqualizerFabric.pdb DESTINATION bin COMPONENT lib CONFIGURATIONS Debug)
  install(FILES ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/EqualizerFabric.pdb DESTINATION bin COMPONENT lib CONFIGURATIONS RelWithDebInfo)
endif()

  install(TARGETS EqualizerFabric
        ARCHIVE DESTINATION lib COMPONENT eqdev
        RUNTIME DESTINATION bin COMPONENT eqlib
        LIBRARY DESTINATION lib COMPONENT eqlib)

source_group(\\ REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(fabric FILES ${EQ_FABRIC_PUBLIC_HEADERS} ${EQ_FABRIC_HEADERS}
             ${EQ_FABRIC_SOURCES})
