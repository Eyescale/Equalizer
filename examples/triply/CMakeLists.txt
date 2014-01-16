
include(files.cmake)

add_definitions(-DPLYLIB_SHARED)
add_library(triply SHARED ${LIB_SOURCES} ${PUBLIC_HEADERS} ${LIB_HEADERS})

set_target_properties(triply
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

target_link_libraries(triply ${COLLAGE_LIBRARIES} ${EQ_GL_LIBRARIES}
  ${GLEW_LIBRARY})

install(FILES ${PUBLIC_HEADERS} DESTINATION include/triply COMPONENT dev)
install(TARGETS triply
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)
