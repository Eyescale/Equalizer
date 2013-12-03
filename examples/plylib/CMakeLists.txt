include(files.cmake)

add_library(plylib SHARED ${LIB_SOURCES} ${PUBLIC_HEADERS} ${LIB_HEADERS})
set_target_properties(plylib  PROPERTIES VERSION ${VERSION}
                                         SOVERSION ${VERSION_ABI}
                                         LINK_FLAGS "-Wl,--no-undefined")

target_link_libraries( plylib ${EQUALIZER_LIBRARIES} )

# Install headers
install(FILES ${PUBLIC_HEADERS} DESTINATION include/plylib COMPONENT dev)

# Install library
install(TARGETS plylib
        ARCHIVE DESTINATION lib COMPONENT dev
        RUNTIME DESTINATION bin COMPONENT lib
        LIBRARY DESTINATION lib COMPONENT lib)