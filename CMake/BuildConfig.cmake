# Installs CMakeCache.txt, environment and information about git repository

include(Revision)

install (PROGRAMS "${CMAKE_BINARY_DIR}/CMakeCache.txt" DESTINATION
  share/${CMAKE_PROJECT_NAME}/CMake/BuildParams)

execute_process(COMMAND env OUTPUT_VARIABLE environment)
# TODO: write ${environment} and ${GIT_FOO} variables to file and install it
