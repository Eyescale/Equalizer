# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/examples)
list(APPEND CPPCHECK_EXTRA_ARGS -I${PROJECT_SOURCE_DIR}/examples)

set(EQPLYCONVERTER_SOURCES main.cpp)
set(EQPLYCONVERTER_LINK_LIBRARIES Equalizer triply)
common_application(eqPlyConverter)
