# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

list(APPEND CPPCHECK_EXTRA_ARGS --suppress=invalidscanf
  --suppress=invalidscanf_libc
  --suppress=variableScope --suppress=invalidPointerCast
  --suppress=invalidPrintfArgType_sint) # Yes, it's that bad.

set(EVOLVECONVERTER_HEADERS codebase.h ddsbase.h eVolveConverter.h hlp.h)
set(EVOLVECONVERTER_SOURCES eVolveConverter.cpp ddsbase.cpp)
set(EVOLVECONVERTER_LINK_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY})
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
common_application(eVolveConverter)
