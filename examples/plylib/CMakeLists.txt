
include(files.cmake)

add_definitions(-DPLYLIB_SHARED)
add_library(plylib SHARED ${LIB_SOURCES} ${PUBLIC_HEADERS} ${LIB_HEADERS})

set_target_properties(plylib
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

target_link_libraries( plylib ${EQUALIZER_LIBRARIES} )

install(FILES ${PUBLIC_HEADERS} DESTINATION include/plylib COMPONENT dev)
install(TARGETS plylib
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)
