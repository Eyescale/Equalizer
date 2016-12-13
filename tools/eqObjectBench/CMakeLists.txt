# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/examples)
list(APPEND CPPCHECK_EXTRA_ARGS -I${PROJECT_SOURCE_DIR}/examples)

set(EQOBJECTBENCH_SOURCES
  Client.cpp
  Client.h
  MemModel.h
  MemModel.cpp
  Model.h
  PlyModel.h
  PlyModel.cpp
  Server.cpp
  Server.h
  hwsd.h
  main.cpp
  protocol.h
  )

set(EQOBJECTBENCH_LINK_LIBRARIES PRIVATE Collage triply
  ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(HWSD_FOUND)
  add_definitions(-DEQOBJECTBENCH_USE_HWSD)
  set(_hwsd_components hwsd_net_sys)
  if(SERVUS_USE_DNSSD OR SERVUS_USE_AVAHI_CLIENT)
    list(APPEND _hwsd_components hwsd_gpu_dns_sd hwsd_net_dns_sd)
  endif()
  list(APPEND EQOBJECTBENCH_LINK_LIBRARIES PRIVATE ${_hwsd_components} hwsd_net_dns_sd)
  foreach(_component ${_hwsd_components})
    add_definitions(-DEQOBJECTBENCH_USE_${_component})
  endforeach()
endif()

common_application(eqObjectBench)
