
include(files.cmake)

add_definitions(-DPLYLIB_SHARED)
add_library(ply SHARED ${LIB_SOURCES} ${PUBLIC_HEADERS} ${LIB_HEADERS})

set_target_properties(ply
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

target_link_libraries(ply ${EQUALIZER_LIBRARIES} )

install(FILES ${PUBLIC_HEADERS} DESTINATION include/ply COMPONENT dev)
install(TARGETS ply
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)
