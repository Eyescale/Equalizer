# Copyright (c) 2013 Stefan Eilemann <eile@eyescale.ch>
#
# - Try to find 0MQ C++ bindings
# This module defines
#
# ZMQCPP_FOUND
# ZMQCPP_INCLUDE_DIRS

find_path(ZMQCPP_INCLUDE_DIR zmq.hpp)

set(ZMQCPP_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(zmqcpp DEFAULT_MSG
                                 ZMQCPP_INCLUDE_DIR
)

mark_as_advanced(ZMQCPP_INCLUDE_DIR)
