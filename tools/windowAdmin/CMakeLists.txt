# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/examples/include)
list(APPEND CPPCHECK_EXTRA_ARGS -I${PROJECT_SOURCE_DIR}/examples/include)

set(EQWINDOWADMIN_SOURCES main.cpp)
set(EQWINDOWADMIN_LINK_LIBRARIES EqualizerAdmin)
common_application(eqWindowAdmin)
