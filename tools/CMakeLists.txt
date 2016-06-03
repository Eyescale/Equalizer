# Copyright (c) 2013-2016 Stefan.Eilemann@epfl.ch

add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers

add_subdirectory(affinityCheck)
add_subdirectory(threadAffinity)
add_subdirectory(eqPlyConverter)
add_subdirectory(windowAdmin)
add_subdirectory(server)
add_subdirectory(eVolveConverter)
