
set(HEADERS)
set(SOURCES
  gpu-sd.cpp)

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu-sd ${SOURCES} ${HEADERS})
# target_link_libraries(gpu-sd )
install(TARGETS gpu-sd DESTINATION bin COMPONENT deamon)
